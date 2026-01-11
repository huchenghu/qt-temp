#include "qtutils/log_manager.h"
#include "qtutils/common_utils.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QMessageLogContext>
#include <QThread>
#include <iostream>

namespace QtUtils
{

LogManager &LogManager::instance()
{
  static LogManager instance;
  return instance;
}

LogManager::LogManager()
    : original_qt_msg_handler(nullptr)
{
}

LogManager::~LogManager()
{
  shutdown();
}

bool LogManager::initialize(const QString &log_dir,
                            QtMsgType min_level,
                            bool enable_console,
                            bool enable_file)
{
  if (thread_is_running_)
  {
    qWarning() << "LogManager is already initialized";
    return true;
  }

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
    qWarning("Failed to get log directory");
    return false;
  }

  QDir dir(log_dir_);
  if (!dir.exists() && !dir.mkpath("."))
  {
    qWarning("Failed to create log directory: %s", qPrintable(log_dir_));
    return false;
  }

  cleanupOldLogs();

  original_qt_msg_handler = qInstallMessageHandler(qtMessageHandler);

  worker_thread_ = QThread::create(
      [this]()
      {
        workerThread();
      });
  worker_thread_->start();
  thread_is_running_ = true;

  qDebug() << "LogManager initialized. Log directory:" << log_dir_;
  return true;
}

void LogManager::shutdown()
{
  if (!thread_is_running_)
  {
    return;
  }

  thread_is_running_ = false;
  log_queue_condition_.notify_all();

  if (worker_thread_ != nullptr)
  {
    worker_thread_->quit();
    if (!worker_thread_->wait(3000))
    {
      worker_thread_->terminate();
      worker_thread_->wait();
    }
    delete worker_thread_;
    worker_thread_ = nullptr;
  }

  if (original_qt_msg_handler != nullptr)
  {
    qInstallMessageHandler(original_qt_msg_handler);
    original_qt_msg_handler = nullptr;
  }

  closeLogFile();
}

void LogManager::qtMessageHandler(QtMsgType type,
                                  const QMessageLogContext &context,
                                  const QString &msg)
{
  LogManager &instance = LogManager::instance();

  if (type < instance.min_level_)
  {
    return;
  }

  QString filename = (context.file != nullptr)
                       ? QFileInfo(context.file).fileName()
                       : "unknown";
  QString function =
      (context.function != nullptr) ? QString(context.function) : "unknown";

  LogEntry entry{QDateTime::currentDateTime(),
                 type,
                 filename,
                 context.line,
                 function,
                 msg,
                 reinterpret_cast<qint64>(QThread::currentThreadId())};

  {
    std::lock_guard<std::mutex> lock(instance.log_queue_mutex);
    instance.log_queue_.push(std::move(entry));
  }

  instance.log_queue_condition_.notify_one();

  if (type == QtFatalMsg && (instance.original_qt_msg_handler != nullptr))
  {
    instance.original_qt_msg_handler(type, context, msg);
  }
}

void LogManager::workerThread()
{
  while (thread_is_running_ || !log_queue_.empty())
  {
    std::queue<LogEntry> local_queue;

    {
      std::unique_lock<std::mutex> lock(log_queue_mutex);
      log_queue_condition_.wait(lock,
                                [this]()
                                {
                                  return !log_queue_.empty() ||
                                         !thread_is_running_;
                                });

      if (log_queue_.empty())
      {
        continue;
      }

      std::swap(local_queue, log_queue_);
    }

    while (!local_queue.empty())
    {
      const auto &entry = local_queue.front();
      QString formatted = formatLogEntry(entry);

      if (console_enabled_)
      {
        std::cout << formatted.toStdString() << '\n';
      }

      if (file_enabled_)
      {
        if (current_file_size_ >= max_file_size_)
        {
          if (!rotateLogFile())
          {
            qWarning("Failed to rotate log file");
          }
        }

        if ((current_file_ == nullptr) || (current_file_stream_ == nullptr))
        {
          if (!openLogFile())
          {
            qWarning("Failed to open log file");
          }
        }

        if (current_file_stream_ != nullptr)
        {
          *current_file_stream_ << formatted << "\n";
          current_file_stream_->flush();

          current_file_size_ += formatted.size() + 1; // +1 for newline
        }
      }

      local_queue.pop();
    }
  }
}

QString LogManager::formatLogEntry(const LogEntry &entry)
{
  QString thread_id_str =
      QString("%1").arg(entry.threadid, 16, 16, QChar('0')).toUpper();

  QString formatted =
      QString("[%1] [%2] [%3:%4] [%5] %6")
          .arg(entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"))
          .arg(levelToString(entry.level))
          .arg(entry.file)
          .arg(entry.line)
          .arg(thread_id_str)
          .arg(entry.message);

  return formatted;
}

QString LogManager::levelToString(QtMsgType type)
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

  current_file_ = new QFile(current_file_name_);
  if (!current_file_->open(QIODevice::WriteOnly | QIODevice::Append |
                           QIODevice::Text))
  {
    delete current_file_;
    current_file_ = nullptr;
    return false;
  }

  current_file_stream_ = new QTextStream(current_file_);

  current_file_size_ = current_file_->size();

  qDebug() << "Opened log file:" << current_file_name_;
  return true;
}

void LogManager::closeLogFile()
{
  if (current_file_stream_ != nullptr)
  {
    delete current_file_stream_;
    current_file_stream_ = nullptr;
  }

  if (current_file_ != nullptr)
  {
    current_file_->close();
    delete current_file_;
    current_file_ = nullptr;
  }
}

bool LogManager::rotateLogFile()
{
  qDebug() << "Rotating log file due to size limit";

  closeLogFile();

  QString new_file_name = generateLogFileName();

  if (QFile::exists(current_file_name_))
  {
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    QString rotated_name =
        QString("%1-%2.log")
            .arg(current_file_name_.left(current_file_name_.lastIndexOf('.')))
            .arg(timestamp);

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
      log_dir.entryInfoList(name_filters,
                            QDir::Files | QDir::NoDotAndDotDot,
                            QDir::Time | QDir::Reversed); // 旧文件在前

  qsizetype files_to_delete = files.size() - max_files_count_;
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
  QString datetime_str =
      QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
  QString file_name =
      QString("%1/%2-%3.log").arg(log_dir_).arg(app_name).arg(datetime_str);

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
