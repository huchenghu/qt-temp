#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "app_config.h"
#include "qtutils/log_manager.h"
#include <QDebug>
#include <QPixmap>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow)
{
  ui_->setupUi(this);

  initMainWindow();
}

MainWindow::~MainWindow() = default;

void MainWindow::initMainWindow()
{
  setWindowTitle(AppConfig::windowTitle());

  if (AppConfig::windowMaximized())
  {
    setWindowState(Qt::WindowMaximized);
    // setWindowState(Qt::WindowFullScreen);
    // setWindowFlags(Qt::FramelessWindowHint); // | Qt::WindowStaysOnTopHint
  }

  QPixmap pixmap(AppConfig::imagePath());

  if (!pixmap.isNull())
  {
    ui_->lbl_image->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    ui_->lbl_image->setAlignment(Qt::AlignCenter);
    qDebug() << "image loaded successfully";
  }
  else
  {
    qWarning() << "failed to load image";
  }

  QObject::connect(ui_->btn_exit,
                   &QPushButton::clicked,
                   this,
                   [this]()
                   {
                     qWarning() << "exit button clicked";
                     this->close();
                   });
}
