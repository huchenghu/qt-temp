#pragma once

#include "qtutils/config_manager.h"
#include <algorithm>

namespace AppConfig
{

inline void initDefaults()
{
  QtUtils::ConfigManager::instance().registerDefaults({
      {QStringLiteral("ui/window_title"),     QStringLiteral("Test")                  },
      {QStringLiteral("ui/window_maximized"), false                                   },
      {QStringLiteral("ui/image_path"),       QStringLiteral(":/images/pandas-waving")},
      {QStringLiteral("log/level"),           0                                       },
      {QStringLiteral("log/enabled"),         true                                    },
  });
}

inline QString windowTitle()
{
  return QtUtils::ConfigManager::instance().stringValue(QStringLiteral("ui/window_title"));
}

inline bool windowMaximized()
{
  return QtUtils::ConfigManager::instance().boolValue(QStringLiteral("ui/window_maximized"));
}

inline QString imagePath()
{
  return QtUtils::ConfigManager::instance().stringValue(QStringLiteral("ui/image_path"));
}

inline int logLevel()
{
  return std::clamp(QtUtils::ConfigManager::instance().intValue(QStringLiteral("log/level")), 0, 4);
}

inline bool logEnabled()
{
  return QtUtils::ConfigManager::instance().boolValue(QStringLiteral("log/enabled"));
}

} // namespace AppConfig
