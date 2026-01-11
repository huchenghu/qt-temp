#include "mainwindow.h"
#include "qtutils/config_manager.h"
#include "qtutils/log_manager.h"
#include <QApplication>
#include <QtCore>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QtUtils::LogManager::instance().initialize();
  ConfigManager::instance();

  MainWindow mainwindow;
  mainwindow.show();

  return QApplication::exec();
}
