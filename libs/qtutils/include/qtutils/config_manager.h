#pragma once

#include <QtCore>

class ConfigManager final
{
public:
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(const ConfigManager &) = delete;

  static ConfigManager &instance();

  QVariant value(const QString &key,
                 const QVariant &default_value = QVariant()) const;
  void setValue(const QString &key, const QVariant &value);
  bool save();

  QString stringValue(const QString &key,
                      const QString &default_value = QString()) const;
  int intValue(const QString &key, int default_value = 0) const;
  double doubleValue(const QString &key, double default_value = 0) const;
  bool boolValue(const QString &key, bool default_value = false) const;

  QString configFilePath() const;

  bool logEnabled() const;
  int logLevel() const;
  QString uiWindowTitle() const;
  bool uiWindowMaximized() const;
  QString uiImagePath() const;

private:
  explicit ConfigManager();
  ~ConfigManager() = default;

  void initialize();

  QScopedPointer<QSettings> settings_;
};
