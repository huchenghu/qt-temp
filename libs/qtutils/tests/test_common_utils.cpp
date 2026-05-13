#include "qtutils/common_utils.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTest>

class TestCommonUtils : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  void testGetAppName();
  void testGetAppConfigDirPath();
  void testGetAppLogDirPath();
  void testGetAvailableDiskSpaceInMB();
  void testIsDiskSpaceAvailable();

private:
  QString original_app_name_;
};

void TestCommonUtils::initTestCase()
{
  original_app_name_ = QCoreApplication::applicationName();
  QCoreApplication::setApplicationName(QStringLiteral("test-common-utils"));
}

void TestCommonUtils::cleanupTestCase()
{
  QCoreApplication::setApplicationName(original_app_name_);
}

void TestCommonUtils::testGetAppName()
{
  const QString &app_name = QtUtils::CommonUtils::getAppName();
  QCOMPARE(app_name, QStringLiteral("test-common-utils"));
}

void TestCommonUtils::testGetAppConfigDirPath()
{
  const QString &config_path = QtUtils::CommonUtils::getAppConfigDirPath();
  QVERIFY(!config_path.isEmpty());
  QVERIFY(config_path.contains(QStringLiteral("test-common-utils")));
}

void TestCommonUtils::testGetAppLogDirPath()
{
  const QString &log_path = QtUtils::CommonUtils::getAppLogDirPath();
  QVERIFY(!log_path.isEmpty());
  QVERIFY(log_path.contains(QStringLiteral("test-common-utils")));
}

void TestCommonUtils::testGetAvailableDiskSpaceInMB()
{
  qint64 space = QtUtils::CommonUtils::getAvailableDiskSpaceInMB(QDir::currentPath());
  QVERIFY(space >= 0);
}

void TestCommonUtils::testIsDiskSpaceAvailable()
{
  bool available = QtUtils::CommonUtils::isDiskSpaceAvailable(QDir::currentPath(), 1);
  QVERIFY(available);

  available = QtUtils::CommonUtils::isDiskSpaceAvailable(QDir::currentPath(), 100000000);
  QVERIFY(!available);
}

QTEST_MAIN(TestCommonUtils)
#include "test_common_utils.moc"
