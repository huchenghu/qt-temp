#include "qtutils/common_utils.h"
#include "qtutils/log_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTest>
#include <QThread>
#include <atomic>

class TestLogManager : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  void testSingleton();
  void testDefaults();
  void testConfigure();
  void testLevelFiltering();
  void testFileOutput();

private:
  QString original_app_name_;
  QString log_dir_;
};

void TestLogManager::initTestCase()
{
  original_app_name_ = QCoreApplication::applicationName();
  QCoreApplication::setApplicationName(QStringLiteral("test-log-manager"));

  log_dir_ = QtUtils::CommonUtils::getAppLogDirPath();

  QDir dir(log_dir_);
  if (dir.exists())
  {
    for (const QFileInfo &fi : dir.entryInfoList(QStringList{"*.log"}, QDir::Files))
    {
      QFile::remove(fi.absoluteFilePath());
    }
  }

  QtUtils::LogManager::instance();
}

void TestLogManager::cleanupTestCase()
{
  QDir dir(log_dir_);
  if (dir.exists())
  {
    for (const QFileInfo &fi : dir.entryInfoList(QStringList{"*.log"}, QDir::Files))
    {
      QFile::remove(fi.absoluteFilePath());
    }
  }
  QCoreApplication::setApplicationName(original_app_name_);
}

void TestLogManager::testSingleton()
{
  QtUtils::LogManager &a = QtUtils::LogManager::instance();
  QtUtils::LogManager &b = QtUtils::LogManager::instance();
  QCOMPARE(&a, &b);
}

void TestLogManager::testDefaults()
{
  QtUtils::LogManager &log = QtUtils::LogManager::instance();
  QCOMPARE(log.minLevel(), QtDebugMsg);
  QVERIFY(log.isConsoleEnabled());
  QVERIFY(log.isFileEnabled());
  QCOMPARE(log.currentFileSize(), 0);
  QVERIFY(log.fileCount() >= 0);
}

void TestLogManager::testConfigure()
{
  QtUtils::LogManager &log = QtUtils::LogManager::instance();

  log.configure(QtWarningMsg, false, true);
  QCOMPARE(log.minLevel(), QtWarningMsg);
  QVERIFY(!log.isConsoleEnabled());
  QVERIFY(log.isFileEnabled());

  log.configure(QtDebugMsg, true, false);
  QCOMPARE(log.minLevel(), QtDebugMsg);
  QVERIFY(log.isConsoleEnabled());
  QVERIFY(!log.isFileEnabled());

  log.setMinLevel(QtCriticalMsg);
  QCOMPARE(log.minLevel(), QtCriticalMsg);

  log.setConsoleEnabled(false);
  QVERIFY(!log.isConsoleEnabled());

  log.setFileEnabled(true);
  QVERIFY(log.isFileEnabled());
}

void TestLogManager::testLevelFiltering()
{
  QtUtils::LogManager &log = QtUtils::LogManager::instance();
  log.configure(QtWarningMsg, true, true);

  qDebug() << "should be filtered";
  qWarning() << "should appear";
  qCritical() << "should also appear";
}

void TestLogManager::testFileOutput()
{
  QtUtils::LogManager &log = QtUtils::LogManager::instance();
  log.configure(QtDebugMsg, true, true);

  const int num_threads = 4;
  const int msgs_per_thread = 100;
  std::atomic<int> write_count{0};
  std::atomic<bool> start_flag{false};

  QThread *threads[num_threads];
  for (int t = 0; t < num_threads; ++t)
  {
    threads[t] = QThread::create(
        [&write_count, &start_flag, t]()
        {
          while (!start_flag.load(std::memory_order_acquire))
          {
            QThread::yieldCurrentThread();
          }
          for (int i = 0; i < msgs_per_thread; ++i)
          {
            qDebug() << "concurrent thread" << t << "msg" << i;
            write_count.fetch_add(1, std::memory_order_release);
          }
        });
  }

  for (int t = 0; t < num_threads; ++t)
  {
    threads[t]->start();
  }

  start_flag.store(true, std::memory_order_release);

  for (int t = 0; t < num_threads; ++t)
  {
    QVERIFY(threads[t]->wait(30000));
    delete threads[t];
  }

  QCOMPARE(write_count.load(std::memory_order_acquire), num_threads * msgs_per_thread);

  log.shutdown();

  QString log_file = log.currentLogFile();
  QVERIFY(!log_file.isEmpty());
  QVERIFY(QFileInfo::exists(log_file));
  QVERIFY(QFileInfo(log_file).size() > 0);

  QFile file(log_file);
  QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
  QString content = QString::fromUtf8(file.readAll());
  file.close();

  QVERIFY(content.contains("[DEBUG]"));
  QVERIFY(content.contains("[WARNING]"));
  QVERIFY(content.contains("concurrent thread"));
  int matched_lines = 0;
  for (const QString &line : content.split('\n'))
  {
    if (line.contains("concurrent thread"))
    {
      ++matched_lines;
    }
  }
  QCOMPARE(matched_lines, num_threads * msgs_per_thread);
}

QTEST_MAIN(TestLogManager)
#include "test_log_manager.moc"
