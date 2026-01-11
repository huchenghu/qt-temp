#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qtutils/config_manager.h"
#include "qtutils/log_manager.h"
#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QWidget>
#include <QtCore>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  initMainWindow();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::initMainWindow()
{
  setWindowTitle(ConfigManager::instance().uiWindowTitle());

  if (ConfigManager::instance().uiWindowMaximized())
  {
    setWindowState(Qt::WindowMaximized);
    // setWindowState(Qt::WindowFullScreen);
    // setWindowFlags(Qt::FramelessWindowHint); // | Qt::WindowStaysOnTopHint
  }

  QPixmap pixmap(ConfigManager::instance().uiImagePath());

  if (!pixmap.isNull())
  {
    ui->lbl_image->setPixmap(pixmap.scaled(
        300, 300, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    ui->lbl_image->setAlignment(Qt::AlignCenter);
    qDebug() << "image loaded successfully";
  }
  else
  {
    qWarning() << "failed to load image";
  }

  QObject::connect(ui->btn_exit,
                   &QPushButton::clicked,
                   this,
                   [this]()
                   {
                     qWarning() << "exit button clicked";
                     this->close();
                   });
}
