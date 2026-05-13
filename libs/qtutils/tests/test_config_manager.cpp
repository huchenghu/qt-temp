#include "qtutils/common_utils.h"
#include "qtutils/config_manager.h"
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QTest>

class TestConfigManager : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  void init();

  void testInstance();
  void testValue();
  void testSetValue();
  void testStringValue();
  void testIntValue();
  void testDoubleValue();
  void testBoolValue();
  void testConfigFilePath();
  void testSave();
  void testRemove();
  void testPersistence();

private:
  QString original_app_name_;
  QString config_file_path_;
};

void TestConfigManager::initTestCase()
{
  original_app_name_ = QCoreApplication::applicationName();
  QCoreApplication::setApplicationName(QStringLiteral("test-config-manager"));

  config_file_path_ = QtUtils::CommonUtils::getAppConfigDirPath() + QDir::separator() + QStringLiteral("config.ini");
  QFile::remove(config_file_path_);
}

void TestConfigManager::cleanupTestCase()
{
  if (!config_file_path_.isEmpty())
  {
    QFile::remove(config_file_path_);
  }
  QCoreApplication::setApplicationName(original_app_name_);
}

void TestConfigManager::init()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.remove(QStringLiteral("test/key"));
  config.remove(QStringLiteral("test/int_key"));
  config.remove(QStringLiteral("test/number"));
  config.remove(QStringLiteral("test/double"));
  config.remove(QStringLiteral("test/bool"));
  config.remove(QStringLiteral("test/save_key"));
  config.remove(QStringLiteral("test/remove_key"));
  config.remove(QStringLiteral("test/persist"));
  config.remove(QStringLiteral("test/external"));
}

void TestConfigManager::testInstance()
{
  QtUtils::ConfigManager &instance1 = QtUtils::ConfigManager::instance();
  QtUtils::ConfigManager &instance2 = QtUtils::ConfigManager::instance();
  QCOMPARE(&instance1, &instance2);
}

void TestConfigManager::testValue()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  QVariant value = config.value(QStringLiteral("test/key"), QStringLiteral("default"));
  QCOMPARE(value.toString(), QStringLiteral("default"));

  config.setValue(QStringLiteral("test/key"), QStringLiteral("test_value"));
  value = config.value(QStringLiteral("test/key"));
  QCOMPARE(value.toString(), QStringLiteral("test_value"));
}

void TestConfigManager::testSetValue()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.setValue(QStringLiteral("test/int_key"), 42);
  QCOMPARE(config.intValue(QStringLiteral("test/int_key")), 42);
}

void TestConfigManager::testStringValue()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  QString result = config.stringValue(QStringLiteral("nonexistent"), QStringLiteral("fallback"));
  QCOMPARE(result, QStringLiteral("fallback"));
}

void TestConfigManager::testIntValue()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.setValue(QStringLiteral("test/number"), 123);
  int result = config.intValue(QStringLiteral("test/number"));
  QCOMPARE(result, 123);

  int default_result = config.intValue(QStringLiteral("nonexistent"), 999);
  QCOMPARE(default_result, 999);
}

void TestConfigManager::testDoubleValue()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.setValue(QStringLiteral("test/double"), 3.14);
  double result = config.doubleValue(QStringLiteral("test/double"));
  QCOMPARE(result, 3.14);

  double default_result = config.doubleValue(QStringLiteral("nonexistent"), 2.71);
  QCOMPARE(default_result, 2.71);
}

void TestConfigManager::testBoolValue()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.setValue(QStringLiteral("test/bool"), true);
  bool result = config.boolValue(QStringLiteral("test/bool"));
  QCOMPARE(result, true);

  bool default_result = config.boolValue(QStringLiteral("nonexistent"), false);
  QCOMPARE(default_result, false);
}

void TestConfigManager::testConfigFilePath()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  QString path = config.configFilePath();
  QVERIFY(!path.isEmpty());
  QVERIFY(path.contains(QStringLiteral("config.ini")));
}

void TestConfigManager::testSave()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  QVERIFY(config.save());

  config.setValue(QStringLiteral("test/save_key"), QStringLiteral("saved_value"));
  QVERIFY(config.save());

  QSettings settings(config_file_path_, QSettings::IniFormat);
  QCOMPARE(settings.value(QStringLiteral("test/save_key")).toString(), QStringLiteral("saved_value"));
}

void TestConfigManager::testRemove()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.setValue(QStringLiteral("test/remove_key"), 42);
  QCOMPARE(config.intValue(QStringLiteral("test/remove_key")), 42);

  config.remove(QStringLiteral("test/remove_key"));
  QCOMPARE(config.intValue(QStringLiteral("test/remove_key"), -1), -1);
}

void TestConfigManager::testPersistence()
{
  QtUtils::ConfigManager &config = QtUtils::ConfigManager::instance();
  config.setValue(QStringLiteral("test/persist"), QStringLiteral("stays"));
  config.save();

  QSettings settings(config_file_path_, QSettings::IniFormat);
  QCOMPARE(settings.value(QStringLiteral("test/persist")).toString(), QStringLiteral("stays"));

  settings.setValue(QStringLiteral("test/external"), QStringLiteral("external_write"));
  settings.sync();

  QCOMPARE(config.stringValue(QStringLiteral("test/external")), QStringLiteral("external_write"));
}

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
