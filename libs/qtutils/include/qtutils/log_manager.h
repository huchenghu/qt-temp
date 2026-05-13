#pragma once

#include <QByteArray>
#include <QFile>
#include <QMessageLogContext>
#include <QString>
#include <QThread>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

namespace QtUtils
{

class LogManager final
{
public:
  LogManager(const LogManager &) = delete;
  LogManager &operator=(const LogManager &) = delete;

  static LogManager &instance();

  void shutdown();

  void configure(QtMsgType min_level = QtDebugMsg, bool enable_console = true, bool enable_file = true);

  void setMinLevel(QtMsgType level);
  QtMsgType minLevel() const;

  void setConsoleEnabled(bool enabled);
  bool isConsoleEnabled() const;

  void setFileEnabled(bool enabled);
  bool isFileEnabled() const;

  QString currentLogFile() const;
  qint64 currentFileSize() const;
  qsizetype fileCount() const;

private:
  explicit LogManager();
  ~LogManager();

  struct LogEntry
  {
    qint64 timestamp_ms;
    QtMsgType level;
    QByteArray file;
    int line;
    QByteArray function;
    QByteArray message;
    quintptr threadid;
  };

  static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
  static const char *extractFileName(const char *path);

  bool initialize(const QString &log_dir, QtMsgType min_level, bool enable_console, bool enable_file);

  void workerThread();
  static QString formatLogEntry(const LogEntry &entry);
  static const char *levelToString(QtMsgType type);

  bool openLogFile();
  void closeLogFile();
  bool rotateLogFile();
  void cleanupOldLogs();
  QString generateLogFileName() const;
  void writeLogEntry(const LogEntry &entry, QByteArray &file_batch, QByteArray &console_batch);
  void flushBatches(const QByteArray &file_batch, const QByteArray &console_batch);

  QtMessageHandler original_qt_msg_handler_{nullptr};

  QThread *worker_thread_{nullptr};
  std::atomic<bool> thread_is_running_{false};
  std::atomic<bool> console_enabled_{true};
  std::atomic<bool> file_enabled_{true};
  std::atomic<QtMsgType> min_level_{QtDebugMsg};

  QString log_dir_;
  std::unique_ptr<QFile> current_file_;
  QString current_file_name_;
  std::atomic<qint64> current_file_size_{0};

  std::atomic<qint64> max_file_size_{10 * 1024 * 1024};
  std::atomic<int> max_files_count_{100};

  std::atomic<qint64> flush_size_{8 * 1024};
  std::atomic<int> flush_timeout_ms_{200};

  std::deque<LogEntry> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

} // namespace QtUtils
