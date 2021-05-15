// Qt5
#include <QFileDialog>
// Internal
#include "Scene.hpp"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) noexcept :
  QMainWindow(parent), m_pUI(std::make_unique<Ui::MainWindow>()) {
  m_pUI->setupUi(this);
  m_pScene = new Scene(m_pUI->openGLWidget);
  connect(m_pUI->actionEuRoC, &QAction::triggered,
          [this]() {
    const auto ymlFileName = QFileDialog::getOpenFileName(this, "Get EuRoc yml file", QDir::homePath());
    if(ymlFileName.isEmpty()) {
      return;
    }
  });

  // m_pUI->startPushButton
  // m_pUI->restartPushButton
  // m_pUI->actionEuRoC
  // connect(&QPushButton::click,
}

MainWindow::~MainWindow() noexcept = default;

