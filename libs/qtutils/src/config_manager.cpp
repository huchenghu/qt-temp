#include "qtutils/config_manager.h"
#include "qtutils/common_utils.h"

namespace QtUtils
{

ConfigManager &ConfigManager::instance()
{
  static ConfigManager instance;
  return instance;
}

ConfigManager::ConfigManager()
{
  const QString &config_dir_path = CommonUtils::getAppConfigDirPath();
  if (config_dir_path.isEmpty())
  {
    qWarning("Cannot create config directory");
    return;
  }

  QString config_file = config_dir_path + QDir::separator() + QStringLiteral("config.ini");
  settings_.reset(new QSettings(config_file, QSettings::IniFormat));

  qDebug("Config file: %s", qPrintable(settings_->fileName()));
}

void ConfigManager::registerDefaults(const QMap<QString, QVariant> &defaults)
{
  for (auto it = defaults.constBegin(); it != defaults.constEnd(); ++it)
  {
    if (!defaults_.contains(it.key()))
    {
      defaults_.insert(it.key(), it.value());
      if (!settings_.isNull() && !settings_->contains(it.key()))
      {
        settings_->setValue(it.key(), it.value());
      }
    }
  }
  if (!settings_.isNull())
  {
    settings_->sync();
  }
}

QVariant ConfigManager::value(const QString &key, const QVariant &default_value) const
{
  if (!settings_.isNull() && settings_->contains(key))
  {
    return settings_->value(key);
  }

  if (default_value.isValid())
  {
    return default_value;
  }

  auto it = defaults_.find(key);
  if (it != defaults_.end())
  {
    return it.value();
  }

  return QVariant();
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
  if (!settings_.isNull())
  {
    settings_->setValue(key, value);
  }
}

void ConfigManager::remove(const QString &key)
{
  if (!settings_.isNull())
  {
    settings_->remove(key);
  }
}

bool ConfigManager::save()
{
  if (settings_.isNull())
  {
    return false;
  }
  settings_->sync();
  return settings_->status() == QSettings::NoError;
}

QString ConfigManager::stringValue(const QString &key, const QString &default_value) const
{
  QVariant val = value(key);
  return val.isValid() ? val.toString() : default_value;
}

int ConfigManager::intValue(const QString &key, int default_value) const
{
  QVariant val = value(key);
  if (!val.isValid())
  {
    return default_value;
  }
  bool ok = false;
  int result = val.toInt(&ok);
  return ok ? result : default_value;
}

double ConfigManager::doubleValue(const QString &key, double default_value) const
{
  QVariant val = value(key);
  if (!val.isValid())
  {
    return default_value;
  }
  bool ok = false;
  double result = val.toDouble(&ok);
  return ok ? result : default_value;
}

bool ConfigManager::boolValue(const QString &key, bool default_value) const
{
  QVariant val = value(key);
  return val.isValid() ? val.toBool() : default_value;
}

QString ConfigManager::configFilePath() const
{
  return !settings_.isNull() ? settings_->fileName() : QString();
}

} // namespace QtUtils
