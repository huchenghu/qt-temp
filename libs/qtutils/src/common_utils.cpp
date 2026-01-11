#include "qtutils/common_utils.h"
#include <QtCore>

namespace QtUtils
{

const QString &CommonUtils::getAppName()
{
  static const QString app_name = []()
  {
    QString name = QCoreApplication::applicationName();
    return name.isEmpty() ? QStringLiteral("app") : name;
  }();
  return app_name;
}

const QString &CommonUtils::getAppConfigDirPath()
{
  static const QString app_config_dir_path = buildAppConfigDirPath();
  return app_config_dir_path;
}

const QString &CommonUtils::getAppLogDirPath()
{
  static const QString app_log_dir_path = buildAppLogDirPath();
  return app_log_dir_path;
}

qint64 CommonUtils::getAvailableDiskSpaceInMB(const QString &path)
{
  QStorageInfo storage(path);
  return (storage.isValid() && storage.isReady())
           ? storage.bytesAvailable() / (1024 * 1024)
           : 0;
}

bool CommonUtils::isDiskSpaceAvailable(const QString &path, qint64 min_space_mb)
{
  return getAvailableDiskSpaceInMB(path) >= min_space_mb;
}

QString CommonUtils::buildAppConfigDirPath()
{
  const QString &app_name = getAppName();
  static QString config_dir_path;

  const QString app_dir_path = QCoreApplication::applicationDirPath();
  if (QFileInfo(app_dir_path).isWritable())
  {
    config_dir_path = app_dir_path + QDir::separator() + app_name + "-configs";
  }
  else
  {
    const QString std_config_path =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    config_dir_path =
        std_config_path.isEmpty()
            ? QDir::currentPath() + QDir::separator() + app_name + "-configs"
            : std_config_path + QDir::separator() + app_name + "-configs";
  }

  QDir config_dir(config_dir_path);
  if (!config_dir.exists() && !config_dir.mkpath("."))
  {
    qWarning() << "failed to create config directory:" << config_dir_path;
    return {};
  }

  return QDir::cleanPath(config_dir.path());
}

QString CommonUtils::buildAppLogDirPath()
{
  const QString &app_name = getAppName();
  QString log_dir_path;

  const QString app_dir_path = QCoreApplication::applicationDirPath();
  if (QFileInfo(app_dir_path).isWritable())
  {
    log_dir_path = app_dir_path + QDir::separator() + app_name + "-logs";
  }
  else
  {
    const QString std_cache_path =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    log_dir_path =
        std_cache_path.isEmpty()
            ? QDir::currentPath() + QDir::separator() + app_name + "-logs"
            : std_cache_path + QDir::separator() + app_name + "-logs";
  }

  QDir log_dir(log_dir_path);
  if (!log_dir.exists() && !log_dir.mkpath("."))
  {
    qWarning() << "failed to create log directory:" << log_dir_path;
    return {};
  }

  return QDir::cleanPath(log_dir.path());
}

} // namespace QtUtils
