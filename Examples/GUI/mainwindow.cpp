// STL
#include <fstream>
#include <sstream>
// Qt5
#include <QDebug>
#include <QFileDialog>
// Internal
#include "Scene.hpp"
#include "mainwindow.hpp"
#include "dialogEuRoC.hpp"
#include "ui_mainwindow.h"


void LoadImages(const std::string &strPathLeft,
                const std::string &strPathRight,
                const std::string &strPathTimes,
                std::vector<std::string> &vstrImageLeft,
                std::vector<std::string> &vstrImageRight,
                std::vector<double> &vTimeStamps) {
  std::ifstream fTimes(strPathTimes);
  if(!fTimes.is_open()) {
    return;
  }

  vTimeStamps.reserve(5000);
  vstrImageLeft.reserve(5000);
  vstrImageRight.reserve(5000);
  while(!fTimes.eof()) {
    std::string s;
    getline(fTimes, s);
    if(!s.empty()) {
      std::stringstream ss;
      ss << s;
      vstrImageLeft.push_back(strPathLeft + "/" + ss.str() + ".png");
      vstrImageRight.push_back(strPathRight + "/" + ss.str() + ".png");
      double t = 0;
      ss >> t;
      vTimeStamps.push_back(t / 1e9);
    }
  }
}

MainWindow::MainWindow(QWidget *parent) noexcept :
  QMainWindow(parent),
  m_pDataset(std::make_unique<DatasetEuRoC>()),
  m_pUI(std::make_unique<Ui::MainWindow>()) {
  m_pUI->setupUi(this);

  m_pScene = new Scene(m_pUI->openGLWidget);
  connect(m_pUI->actionEuRoC, &QAction::triggered,
          [this]() {
    DialogEuRoC dialog(*m_pDataset);
    dialog.exec();

    LoadImages(m_pDataset->cam0.toStdString(),
        m_pDataset->cam1.toStdString(),
        m_pDataset->timestamp.toStdString(),
        vstrImageLeft,
        vstrImageRight,
        vTimeStamp);

    m_pUI->statusbar->showMessage(QString("Found %1").arg(vTimeStamp.size()));
    readyToProcess();
  });

  m_pUI->startPushButton->setEnabled(false);
  m_pUI->restartPushButton->setEnabled(false);
  // m_pUI->startPushButton
  // m_pUI->restartPushButton
  // m_pUI->actionEuRoC
  connect(m_pUI->startPushButton, &QPushButton::click,
          this, &MainWindow::startProcess);
}

MainWindow::~MainWindow() noexcept = default;

void MainWindow::readyToProcess() noexcept {
  m_pUI->startPushButton->setEnabled(true);
  m_pUI->restartPushButton->setEnabled(true);
}

void MainWindow::startProcess() noexcept {
  mWorkerThread = std::thread([this]() {
      process();
  });
}

void MainWindow::process() noexcept {
}

