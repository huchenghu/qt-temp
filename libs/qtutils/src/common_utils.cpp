#include "qtutils/common_utils.h"
#include <QtCore>

namespace QtUtils
{

const QString &CommonUtils::getAppName()
{
  static const QString app_name = []()
  {
    QString name = QCoreApplication::applicationName();
    return name.isEmpty() ? QStringLiteral("default-appname") : name;
  }();
  return app_name;
}

const QString &CommonUtils::getAppRootDirPath()
{
  static const QString path = buildDirPath(getAppName() + "-data", QStandardPaths::HomeLocation);
  return path;
}

const QString &CommonUtils::getAppConfigDirPath()
{
  static const QString path =
      buildDirPath(getAppName() + "-data" + QDir::separator() + "configs", QStandardPaths::HomeLocation);
  return path;
}

const QString &CommonUtils::getAppLogDirPath()
{
  static const QString path =
      buildDirPath(getAppName() + "-data" + QDir::separator() + "logs", QStandardPaths::HomeLocation);
  return path;
}

qint64 CommonUtils::getAvailableDiskSpaceInMB(const QString &path)
{
  QStorageInfo storage(path);
  return (storage.isValid() && storage.isReady()) ? storage.bytesAvailable() / (1024 * 1024) : 0;
}

bool CommonUtils::isDiskSpaceAvailable(const QString &path, qint64 min_space_mb)
{
  return getAvailableDiskSpaceInMB(path) >= min_space_mb;
}

QString CommonUtils::buildDirPath(const QString &subdir, QStandardPaths::StandardLocation std_location)
{
  const QString app_dir = QCoreApplication::applicationDirPath();
  QString base = QFileInfo(app_dir).isWritable() ? app_dir : QStandardPaths::writableLocation(std_location);
  if (base.isEmpty())
  {
    base = QDir::currentPath();
  }

  if (!subdir.isEmpty())
  {
    base += QDir::separator() + subdir;
  }

  QDir dir(base);
  if (!dir.exists() && !dir.mkpath("."))
  {
    qWarning() << "failed to create directory:" << base;
    return {};
  }

  return QDir::cleanPath(dir.path());
}

} // namespace QtUtils
