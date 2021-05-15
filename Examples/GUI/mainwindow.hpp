#pragma once
// STL
#include <thread>
#include <memory>
// Qt5
#include <QMainWindow>
#include <QGLViewer/qglviewer.h>


class Scene;
namespace Ui {
  class MainWindow;
} // namespace Ui

class MainWindow final : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr) noexcept;

  ~MainWindow() noexcept override;

private:
  Scene *m_pScene = nullptr;
  std::thread mWorkerThread;
  std::unique_ptr<Ui::MainWindow> m_pUI;

};

