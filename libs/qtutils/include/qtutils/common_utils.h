#pragma once

#include <QStandardPaths>
#include <QStorageInfo>
#include <QString>

namespace QtUtils
{

class CommonUtils
{
public:
  CommonUtils() = delete;
  CommonUtils(const CommonUtils &) = delete;
  CommonUtils &operator=(const CommonUtils &) = delete;

  static const QString &getAppName();
  static const QString &getAppRootDirPath();
  static const QString &getAppConfigDirPath();
  static const QString &getAppLogDirPath();

  static qint64 getAvailableDiskSpaceInMB(const QString &path);
  static bool isDiskSpaceAvailable(const QString &path, qint64 min_space_mb = 100);

private:
  static QString buildDirPath(const QString &subdir, QStandardPaths::StandardLocation std_location);
};

} // namespace QtUtils
