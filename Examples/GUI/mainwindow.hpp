#pragma once
// STL
#include <thread>
#include <memory>
#include <string>
#include <vector>
// Qt5
#include <QMainWindow>
#include <QGLViewer/qglviewer.h>


class Scene;
struct DatasetEuRoC;
namespace Ui {
  class MainWindow;
} // namespace Ui

namespace ORB_SLAM2 {
  class System;
} // namespace ORB_SLAM2

class MainWindow final : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr) noexcept;

  ~MainWindow() noexcept override;

signals:
  void processImage(int value);

protected:
  void readyToProcess() noexcept;

  void process() noexcept;

  void startProcess() noexcept;

private:
  Scene *m_pScene = nullptr;
  std::unique_ptr<DatasetEuRoC> m_pDataset;
  std::thread mWorkerThread;
  std::unique_ptr<Ui::MainWindow> m_pUI;
  std::unique_ptr<ORB_SLAM2::System> m_pSystem;

  // Retrieve paths to images
  std::vector<std::string> vstrImageLeft;
  std::vector<std::string> vstrImageRight;
  std::vector<double> vTimeStamp;

};

