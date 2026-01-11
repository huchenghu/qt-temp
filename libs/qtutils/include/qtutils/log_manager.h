#pragma once

#include <QtConcurrent>
#include <QtCore>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace QtUtils
{

class LogManager final
{
public:
  LogManager(const LogManager &) = delete;
  LogManager &operator=(const LogManager &) = delete;

  static LogManager &instance();

  bool initialize(const QString &log_dir = QString(),
                  QtMsgType min_level = QtDebugMsg,
                  bool enable_console = true,
                  bool enable_file = true);

  void shutdown();

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
    QDateTime timestamp;
    QtMsgType level;
    QString file;
    int line;
    QString function;
    QString message;
    qint64 threadid;
  };

  static void qtMessageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &msg);

  void workerThread();
  static QString formatLogEntry(const LogEntry &entry);
  static QString levelToString(QtMsgType type);

  bool openLogFile();
  void closeLogFile();
  bool rotateLogFile();
  void cleanupOldLogs();
  QString generateLogFileName() const;

  QtMessageHandler original_qt_msg_handler;

  QThread *worker_thread_{nullptr};
  std::atomic<bool> thread_is_running_{false};
  std::atomic<bool> console_enabled_{true};
  std::atomic<bool> file_enabled_{true};
  std::atomic<QtMsgType> min_level_{QtDebugMsg};

  QString log_dir_;
  QFile *current_file_{nullptr};
  QTextStream *current_file_stream_{nullptr};
  QString current_file_name_;
  std::atomic<qint64> current_file_size_{0};
  std::atomic<int> total_files_count_{0};

  qint64 max_file_size_{10 * 1024 * 1024};
  int max_files_count_{100};

  std::queue<LogEntry> log_queue_;
  mutable std::mutex log_queue_mutex;
  std::condition_variable log_queue_condition_;
};

} // namespace QtUtils
