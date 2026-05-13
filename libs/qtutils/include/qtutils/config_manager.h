#pragma once

#include <QMap>
#include <QScopedPointer>
#include <QSettings>
#include <QString>
#include <QVariant>

namespace QtUtils
{

class ConfigManager final
{
public:
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(const ConfigManager &) = delete;

  static ConfigManager &instance();

  void registerDefaults(const QMap<QString, QVariant> &defaults);

  QVariant value(const QString &key, const QVariant &default_value = QVariant()) const;
  void setValue(const QString &key, const QVariant &value);
  void remove(const QString &key);
  bool save();

  QString stringValue(const QString &key, const QString &default_value = QString()) const;
  int intValue(const QString &key, int default_value = 0) const;
  double doubleValue(const QString &key, double default_value = 0.0) const;
  bool boolValue(const QString &key, bool default_value = false) const;

  QString configFilePath() const;

private:
  explicit ConfigManager();
  ~ConfigManager() = default;

  QScopedPointer<QSettings> settings_;
  QMap<QString, QVariant> defaults_;
};

} // namespace QtUtils
