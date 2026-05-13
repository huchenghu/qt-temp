#include "qtutils/log_manager.h"
#include "qtutils/common_utils.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <chrono>
#include <cstdio>

namespace QtUtils
{

LogManager &LogManager::instance()
{
  static LogManager instance;
  return instance;
}

LogManager::LogManager()
{
  initialize(QString(), QtDebugMsg, true, true);
}

LogManager::~LogManager()
{
  shutdown();
}

bool LogManager::initialize(const QString &log_dir, QtMsgType min_level, bool enable_console, bool enable_file)
{
  min_level_ = min_level;
  console_enabled_ = enable_console;
  file_enabled_ = enable_file;

  if (log_dir.isEmpty())
  {
    log_dir_ = CommonUtils::getAppLogDirPath();
  }
  else
  {
    log_dir_ = log_dir;
  }

  if (log_dir_.isEmpty())
  {
    qWarning("Log directory unavailable, disabling file logging");
    file_enabled_ = false;
  }
  else
  {
    QDir dir(log_dir_);
    if (!dir.exists() && !dir.mkpath("."))
    {
      qWarning("Failed to create log directory: %s, disabling file logging", qPrintable(log_dir_));
      file_enabled_ = false;
    }
    else
    {
      cleanupOldLogs();
    }
  }

  original_qt_msg_handler_ = qInstallMessageHandler(qtMessageHandler);

  worker_thread_ = QThread::create(
      [this]()
      {
        workerThread();
      });
  worker_thread_->start();
  thread_is_running_ = true;
  return true;
}

void LogManager::configure(QtMsgType min_level, bool enable_console, bool enable_file)
{
  min_level_ = min_level;
  console_enabled_ = enable_console;
  if (enable_file && (log_dir_.isEmpty() || !QDir().mkpath(log_dir_)))
  {
    qWarning("Log directory unavailable, file logging remains disabled");
    file_enabled_ = false;
  }
  else
  {
    file_enabled_ = enable_file;
  }
}

void LogManager::shutdown()
{
  if (!thread_is_running_)
  {
    return;
  }

  thread_is_running_ = false;
  cond_.notify_all();

  if (worker_thread_ != nullptr)
  {
    if (!worker_thread_->wait(10000))
    {
      qWarning("LogManager worker thread did not stop gracefully");
    }
    delete worker_thread_;
    worker_thread_ = nullptr;
  }

  if (original_qt_msg_handler_ != nullptr)
  {
    qInstallMessageHandler(original_qt_msg_handler_);
    original_qt_msg_handler_ = nullptr;
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!queue_.empty())
    {
      LogEntry entry = std::move(queue_.front());
      queue_.pop_front();

      QString formatted_qs = formatLogEntry(entry);
      QByteArray bytes = formatted_qs.toUtf8();
      bytes.append('\n');

      if (file_enabled_ && current_file_)
      {
        current_file_->write(bytes);
        current_file_->flush();
      }

      if (console_enabled_)
      {
        fwrite(bytes.constData(), 1, static_cast<size_t>(bytes.size()), stdout);
        fflush(stdout);
      }
    }
  }

  closeLogFile();
}

const char *LogManager::extractFileName(const char *path)
{
  if (path == nullptr)
  {
    return "unknown";
  }

  const char *name = path;
  for (const char *p = path; *p != '\0'; ++p)
  {
    if (*p == '/' || *p == '\\')
    {
      name = p + 1;
    }
  }
  return name;
}

void LogManager::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  LogManager &self = LogManager::instance();

  if (type < self.min_level_)
  {
    return;
  }

  const char *filename = extractFileName(context.file);
  const char *function = (context.function != nullptr) ? context.function : "unknown";
  QByteArray message = msg.toUtf8();
  qint64 timestamp_ms = QDateTime::currentMSecsSinceEpoch();

  LogEntry entry{timestamp_ms,
                 type,
                 QByteArray(filename),
                 context.line,
                 QByteArray(function),
                 message,
                 reinterpret_cast<quintptr>(QThread::currentThreadId())};

  {
    std::unique_lock<std::mutex> lock(self.mutex_);
    self.queue_.push_back(std::move(entry));
  }
  self.cond_.notify_all();

  if (type == QtFatalMsg)
  {
    self.cond_.notify_all();
    if (self.worker_thread_ != nullptr)
    {
      self.worker_thread_->wait(1000);
    }

    if (self.original_qt_msg_handler_ != nullptr)
    {
      self.original_qt_msg_handler_(type, context, msg);
    }
  }
}

void LogManager::workerThread()
{
  using namespace std::chrono;
  while (true)
  {
    std::deque<LogEntry> batch;

    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait_for(lock,
                     milliseconds(flush_timeout_ms_.load()),
                     [this]()
                     {
                       return !queue_.empty() || !thread_is_running_;
                     });

      if (!queue_.empty())
      {
        batch.swap(queue_);
      }
      else if (!thread_is_running_)
      {
        break;
      }
    }

    if (batch.empty())
    {
      continue;
    }

    QByteArray file_batch;
    QByteArray console_batch;

    while (!batch.empty())
    {
      writeLogEntry(batch.front(), file_batch, console_batch);
      batch.pop_front();

      if (static_cast<qint64>(file_batch.size()) >= flush_size_.load())
      {
        flushBatches(file_batch, console_batch);
        file_batch.clear();
        console_batch.clear();
      }
    }

    flushBatches(file_batch, console_batch);
  }
}

void LogManager::writeLogEntry(const LogEntry &entry, QByteArray &file_batch, QByteArray &console_batch)
{
  QString formatted_qs = formatLogEntry(entry);
  QByteArray bytes = formatted_qs.toUtf8();
  bytes.append('\n');

  qint64 max_size = max_file_size_.load();
  if (current_file_size_.load() + static_cast<qint64>(bytes.size()) >= max_size)
  {
    flushBatches(file_batch, console_batch);
    file_batch.clear();
    console_batch.clear();

    if (!rotateLogFile())
    {
      qWarning("Failed to rotate log file");
    }
  }

  if (file_enabled_ && !current_file_)
  {
    if (!openLogFile())
    {
      qWarning("Failed to open log file");
    }
  }

  if (file_enabled_ && current_file_)
  {
    file_batch.append(bytes);
  }

  if (console_enabled_)
  {
    console_batch.append(bytes);
  }
}

void LogManager::flushBatches(const QByteArray &file_batch, const QByteArray &console_batch)
{
  if (file_enabled_ && !file_batch.isEmpty() && current_file_)
  {
    qint64 written = current_file_->write(file_batch);
    if (written > 0)
    {
      current_file_size_ += written;
    }
    current_file_->flush();
  }

  if (console_enabled_ && !console_batch.isEmpty())
  {
    fwrite(console_batch.constData(), 1, static_cast<size_t>(console_batch.size()), stdout);
    fflush(stdout);
  }
}

QString LogManager::formatLogEntry(const LogEntry &entry)
{
  QString thread_id_str = QString("%1").arg(static_cast<qulonglong>(entry.threadid), 16, 16, QChar('0')).toUpper();

  QDateTime ts = QDateTime::fromMSecsSinceEpoch(entry.timestamp_ms);
  QString file_str = QString::fromUtf8(entry.file);
  QString func_str = QString::fromUtf8(entry.function);
  QString msg_str = QString::fromUtf8(entry.message);

  QString formatted = QString("[%1] [%2] [%3:%4] [%5] %6")
                          .arg(ts.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                          .arg(levelToString(entry.level))
                          .arg(file_str)
                          .arg(entry.line)
                          .arg(thread_id_str)
                          .arg(msg_str);

  return formatted;
}

const char *LogManager::levelToString(QtMsgType type)
{
  switch (type)
  {
  case QtDebugMsg:
    return "DEBUG";
  case QtInfoMsg:
    return "INFO";
  case QtWarningMsg:
    return "WARNING";
  case QtCriticalMsg:
    return "ERROR";
  case QtFatalMsg:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

bool LogManager::openLogFile()
{
  closeLogFile();

  if (current_file_name_.isEmpty())
  {
    current_file_name_ = generateLogFileName();
  }

  current_file_ = std::make_unique<QFile>(current_file_name_);
  if (!current_file_->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
  {
    current_file_.reset();
    return false;
  }

  current_file_size_ = current_file_->size();

  qDebug() << "Opened log file:" << current_file_name_;
  return true;
}

void LogManager::closeLogFile()
{
  if (current_file_)
  {
    current_file_->close();
    current_file_.reset();
  }
}

bool LogManager::rotateLogFile()
{
  qDebug() << "Rotating log file due to size limit";

  closeLogFile();

  QString new_file_name = generateLogFileName();

  if (QFile::exists(current_file_name_))
  {
    QString base_name = current_file_name_.left(current_file_name_.lastIndexOf('.'));
    QString rotated_name;
    int suffix = 0;

    do
    {
      QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
      if (suffix == 0)
      {
        rotated_name = QString("%1-%2.log").arg(base_name).arg(timestamp);
      }
      else
      {
        rotated_name = QString("%1-%2-%3.log").arg(base_name).arg(timestamp).arg(suffix);
      }
      ++suffix;
    } while (QFile::exists(rotated_name) && suffix < 100);

    if (!QFile::rename(current_file_name_, rotated_name))
    {
      qWarning("Failed to rename log file: %s", qPrintable(current_file_name_));
    }
  }

  current_file_name_ = new_file_name;

  cleanupOldLogs();

  return openLogFile();
}

void LogManager::cleanupOldLogs()
{
  QDir log_dir(log_dir_);
  QStringList name_filters = {"*.log"};
  QFileInfoList files =
      log_dir.entryInfoList(name_filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

  int max_count = max_files_count_.load();
  qsizetype files_to_delete = files.size() - max_count;
  if (files_to_delete <= 0)
  {
    return;
  }

  for (int i = 0; i < files_to_delete && i < files.size(); ++i)
  {
    QFile::remove(files[i].absoluteFilePath());
    qDebug() << "Deleted old log file:" << files[i].fileName();
  }
}

QString LogManager::generateLogFileName() const
{
  const QString &app_name = CommonUtils::getAppName();
  QString datetime_str = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
  QString file_name = QString("%1/%2-%3.log").arg(log_dir_).arg(app_name).arg(datetime_str);

  return file_name;
}

void LogManager::setMinLevel(QtMsgType level)
{
  min_level_ = level;
}

QtMsgType LogManager::minLevel() const
{
  return min_level_;
}

void LogManager::setConsoleEnabled(bool enabled)
{
  console_enabled_ = enabled;
}

bool LogManager::isConsoleEnabled() const
{
  return console_enabled_;
}

void LogManager::setFileEnabled(bool enabled)
{
  if (enabled && (log_dir_.isEmpty() || !QDir().mkpath(log_dir_)))
  {
    qWarning("Log directory unavailable, file logging remains disabled");
    return;
  }
  file_enabled_ = enabled;
}

bool LogManager::isFileEnabled() const
{
  return file_enabled_;
}

QString LogManager::currentLogFile() const
{
  return current_file_name_;
}

qint64 LogManager::currentFileSize() const
{
  return current_file_size_.load();
}

qsizetype LogManager::fileCount() const
{
  QDir log_dir(log_dir_);
  QStringList name_filters = {"*.log"};
  return log_dir.entryList(name_filters, QDir::Files).size();
}

} // namespace QtUtils
