#include "qtutils/config_manager.h"
#include "qtutils/common_utils.h"

ConfigManager &ConfigManager::instance()
{
  static ConfigManager instance;
  return instance;
}

ConfigManager::ConfigManager()
{
  const QString &config_dir_path = QtUtils::CommonUtils::getAppConfigDirPath();
  if (config_dir_path.isEmpty())
  {
    qWarning("Cannot create config directory");
    return;
  }

  QString config_file =
      config_dir_path + QDir::separator() + QStringLiteral("config.ini");
  settings_.reset(new QSettings(config_file, QSettings::IniFormat));

  qDebug("Config file: %s", qPrintable(settings_->fileName()));

  initialize();
}

void ConfigManager::initialize()
{
  static const QMap<QString, QVariant> defaults = {
      {QStringLiteral("log/enabled"),         true                      },
      {QStringLiteral("log/level"),           0                         },
      {QStringLiteral("ui/window_title"),     QStringLiteral("测试程序")},
      {QStringLiteral("ui/window_maximized"), false                     },
      {QStringLiteral("ui/image_path"),
       QStringLiteral(":/images/pandas-waving")                         }
  };

  bool needs_save = false;
  for (auto it = defaults.constBegin(); it != defaults.constEnd(); ++it)
  {
    if (!settings_->contains(it.key()))
    {
      settings_->setValue(it.key(), it.value());
      needs_save = true;
    }
  }

  if (needs_save)
  {
    settings_->sync();
  }
}

QVariant ConfigManager::value(const QString &key,
                              const QVariant &default_value) const
{
  return settings_->value(key, default_value);
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
  settings_->setValue(key, value);
}

bool ConfigManager::save()
{
  settings_->sync();
  return settings_->status() == QSettings::NoError;
}

QString ConfigManager::stringValue(const QString &key,
                                   const QString &default_value) const
{
  return value(key, default_value).toString();
}

int ConfigManager::intValue(const QString &key, int default_value) const
{
  bool ok = false;
  int result = value(key, default_value).toInt(&ok);
  return ok ? result : default_value;
}

double ConfigManager::doubleValue(const QString &key,
                                  double default_value) const
{
  bool ok = false;
  double result = value(key, default_value).toDouble(&ok);
  return ok ? result : default_value;
}

bool ConfigManager::boolValue(const QString &key, bool default_value) const
{
  return value(key, default_value).toBool();
}

QString ConfigManager::configFilePath() const
{
  return settings_->fileName();
}

bool ConfigManager::logEnabled() const
{
  return boolValue(QStringLiteral("log/enabled"), true);
}

int ConfigManager::logLevel() const
{
  return std::clamp(intValue(QStringLiteral("log/level"), 0), 0, 4);
}

QString ConfigManager::uiWindowTitle() const
{
  return stringValue(QStringLiteral("ui/window_title"),
                     QStringLiteral("Application"));
}

bool ConfigManager::uiWindowMaximized() const
{
  return boolValue(QStringLiteral("ui/window_maximized"), false);
}

QString ConfigManager::uiImagePath() const
{
  return stringValue(QStringLiteral("ui/image_path"),
                     QStringLiteral(":/images/default"));
}
