#include "app_config.h"
#include "mainwindow.h"
#include "qtutils/log_manager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QtUtils::LogManager::instance();
  AppConfig::initDefaults();

  MainWindow mainwindow;
  mainwindow.show();

  return QApplication::exec();
}
