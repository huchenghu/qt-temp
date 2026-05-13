#include "qtutils/log_manager.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

struct BenchConfig
{
  int thread_count{4};
  int logs_per_thread{50000};
  int message_size{128};
  int warmup_logs{1000};
  bool enable_file{true};
  bool enable_console{true};
  bool simulate_lidar{true};
  int burst_count{10};
  int burst_interval_us{1000};
};

struct BenchResult
{
  qint64 total_logs{0};
  qint64 total_bytes{0};
  qint64 elapsed_ms{0};
  double throughput_logs_per_sec{0};
  double throughput_mb_per_sec{0};
  double avg_enqueue_latency_us{0};
};

QString generateLidarPointData(int point_count)
{
  QString data;
  data.reserve(point_count * 32);
  for (int i = 0; i < point_count; ++i)
  {
    double angle = i * 0.36;
    double distance = 5.0 + (i % 100) * 0.1;
    double intensity = 50 + (i % 50);
    data += QString("[%1,%2,%3] ").arg(angle, 0, 'f', 2).arg(distance, 0, 'f', 2).arg(intensity, 0, 'f', 1);
    if (i > 0 && i % 10 == 0)
    {
      data += "\n  ";
    }
  }
  return data;
}

QString generateMessage(int size, int thread_id, int msg_id)
{
  if (size <= 0)
  {
    return QString("T%1 M%2").arg(thread_id).arg(msg_id);
  }

  QString msg = QString("LidarData T%1 M%2: ").arg(thread_id, 3, 10, QChar('0')).arg(msg_id, 6, 10, QChar('0'));

  if (msg.length() < size)
  {
    int remaining = size - msg.length();
    QString payload(remaining, QChar('X'));
    int midpoint = remaining / 2;
    payload[midpoint] = ' ';
    msg += payload;
  }

  return msg;
}

void lidarSimulatorThread(int thread_id,
                          int logs_count,
                          int message_size,
                          int burst_count,
                          int burst_interval_us,
                          std::atomic<int> &counter,
                          std::atomic<qint64> &total_bytes,
                          std::atomic<qint64> &total_latency_us,
                          std::atomic<bool> &start_flag)
{
  while (!start_flag.load(std::memory_order_acquire))
  {
    std::this_thread::yield();
  }

  int remaining = logs_count;
  int burst_size = burst_count;

  while (remaining > 0)
  {
    int batch = std::min(burst_size, remaining);

    for (int i = 0; i < batch; ++i)
    {
      auto t1 = std::chrono::steady_clock::now();

      QString msg;
      if (remaining % 100 == 0)
      {
        msg = QString("Frame %1: %2").arg(logs_count - remaining).arg(generateLidarPointData(50));
      }
      else
      {
        msg = generateMessage(message_size, thread_id, logs_count - remaining);
      }

      total_bytes += msg.toUtf8().size();
      qDebug() << msg;
      ++counter;

      auto t2 = std::chrono::steady_clock::now();
      total_latency_us += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }

    remaining -= batch;

    if (remaining > 0 && burst_interval_us > 0)
    {
      std::this_thread::sleep_for(std::chrono::microseconds(burst_interval_us));
    }
  }
}

void standardBenchThread(int thread_id,
                         int logs_count,
                         int message_size,
                         std::atomic<int> &counter,
                         std::atomic<qint64> &total_bytes,
                         std::atomic<qint64> &total_latency_us,
                         std::atomic<bool> &start_flag)
{
  while (!start_flag.load(std::memory_order_acquire))
  {
    std::this_thread::yield();
  }

  for (int i = 0; i < logs_count; ++i)
  {
    auto t1 = std::chrono::steady_clock::now();

    QString msg = generateMessage(message_size, thread_id, i);
    total_bytes += msg.toUtf8().size();
    qDebug() << msg;
    ++counter;

    auto t2 = std::chrono::steady_clock::now();
    total_latency_us += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
  }
}

void warmup(int count)
{
  for (int i = 0; i < count; ++i)
  {
    qDebug() << "Warmup message" << i;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

BenchResult runBenchmark(const BenchConfig &config)
{
  BenchResult result;
  result.total_logs = static_cast<qint64>(config.thread_count) * config.logs_per_thread;

  std::atomic<int> counter{0};
  std::atomic<qint64> total_bytes{0};
  std::atomic<qint64> total_latency_us{0};
  std::atomic<bool> start_flag{false};

  warmup(config.warmup_logs);

  std::vector<std::thread> threads;
  threads.reserve(config.thread_count);

  for (int t = 0; t < config.thread_count; ++t)
  {
    if (config.simulate_lidar)
    {
      threads.emplace_back(lidarSimulatorThread,
                           t,
                           config.logs_per_thread,
                           config.message_size,
                           config.burst_count,
                           config.burst_interval_us,
                           std::ref(counter),
                           std::ref(total_bytes),
                           std::ref(total_latency_us),
                           std::ref(start_flag));
    }
    else
    {
      threads.emplace_back(standardBenchThread,
                           t,
                           config.logs_per_thread,
                           config.message_size,
                           std::ref(counter),
                           std::ref(total_bytes),
                           std::ref(total_latency_us),
                           std::ref(start_flag));
    }
  }

  auto bench_start = std::chrono::steady_clock::now();
  start_flag.store(true, std::memory_order_release);

  for (auto &th : threads)
  {
    th.join();
  }

  auto bench_end = std::chrono::steady_clock::now();
  result.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(bench_end - bench_start).count();
  result.total_bytes = total_bytes.load();
  result.avg_enqueue_latency_us =
      counter.load() > 0 ? static_cast<double>(total_latency_us.load()) / counter.load() : 0;

  result.throughput_logs_per_sec =
      result.elapsed_ms > 0 ? static_cast<double>(result.total_logs) * 1000.0 / result.elapsed_ms : 0;
  result.throughput_mb_per_sec =
      result.elapsed_ms > 0 ? static_cast<double>(result.total_bytes) / 1024.0 / 1024.0 * 1000.0 / result.elapsed_ms
                            : 0;

  return result;
}

void printResult(const BenchConfig &config, const BenchResult &result)
{
  fprintf(stderr, "\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "LogManager Benchmark Report\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "\n[Configuration]\n");
  fprintf(stderr, "  Threads:          %d\n", config.thread_count);
  fprintf(stderr, "  Logs per thread:  %d\n", config.logs_per_thread);
  fprintf(stderr, "  Message size:     %d bytes\n", config.message_size);
  fprintf(stderr, "  File logging:     %s\n", config.enable_file ? "enabled" : "disabled");
  fprintf(stderr, "  Console logging:  %s\n", config.enable_console ? "enabled" : "disabled");
  fprintf(stderr, "  Lidar simulation: %s\n", config.simulate_lidar ? "enabled" : "disabled");
  if (config.simulate_lidar)
  {
    fprintf(stderr, "  Burst count:      %d\n", config.burst_count);
    fprintf(stderr, "  Burst interval:   %d us\n", config.burst_interval_us);
  }
  fprintf(stderr, "\n[Results]\n");
  fprintf(stderr, "  Total logs:       %lld\n", result.total_logs);
  fprintf(stderr, "  App data:         %.2f MB\n", static_cast<double>(result.total_bytes) / 1024.0 / 1024.0);
  fprintf(stderr, "  Elapsed time:     %lld ms\n", result.elapsed_ms);
  fprintf(stderr, "  Throughput:       %.0f logs/sec\n", result.throughput_logs_per_sec);
  fprintf(stderr, "  App throughput:   %.2f MB/sec\n", result.throughput_mb_per_sec);
  fprintf(stderr, "  Avg enqueue:      %.2f us\n", result.avg_enqueue_latency_us);
  fprintf(stderr, "========================================\n\n");
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("log-bench");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("LogManager benchmark tool for high-frequency logging scenarios");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption threadsOption("threads", "Number of producer threads (default: 4)", "count", "4");
  QCommandLineOption logsOption("logs", "Logs per thread (default: 50000)", "count", "50000");
  QCommandLineOption sizeOption("size", "Message size in bytes (default: 128)", "bytes", "128");
  QCommandLineOption noFileOption("no-file", "Disable file logging");
  QCommandLineOption noConsoleOption("no-console", "Disable console logging");
  QCommandLineOption noLidarOption("no-lidar", "Disable lidar simulation mode");
  QCommandLineOption burstOption("burst", "Logs per burst in lidar mode (default: 10)", "count", "10");
  QCommandLineOption intervalOption("interval", "Burst interval in microseconds (default: 1000)", "us", "1000");

  parser.addOption(threadsOption);
  parser.addOption(logsOption);
  parser.addOption(sizeOption);
  parser.addOption(noFileOption);
  parser.addOption(noConsoleOption);
  parser.addOption(noLidarOption);
  parser.addOption(burstOption);
  parser.addOption(intervalOption);

  parser.process(app);

  BenchConfig config;
  config.thread_count = parser.value(threadsOption).toInt();
  config.logs_per_thread = parser.value(logsOption).toInt();
  config.message_size = parser.value(sizeOption).toInt();
  config.enable_file = !parser.isSet(noFileOption);
  config.enable_console = !parser.isSet(noConsoleOption);
  config.simulate_lidar = !parser.isSet(noLidarOption);
  config.burst_count = parser.value(burstOption).toInt();
  config.burst_interval_us = parser.value(intervalOption).toInt();

  config.thread_count = std::max(1, config.thread_count);
  config.logs_per_thread = std::max(1, config.logs_per_thread);
  config.message_size = std::max(0, config.message_size);
  config.burst_count = std::max(1, config.burst_count);
  config.burst_interval_us = std::max(0, config.burst_interval_us);

  QtUtils::LogManager::instance().configure(QtDebugMsg, config.enable_console, config.enable_file);

  fprintf(stderr, "Starting benchmark...\n");

  BenchResult result = runBenchmark(config);

  QtUtils::LogManager::instance().shutdown();

  printResult(config, result);

  return 0;
}
