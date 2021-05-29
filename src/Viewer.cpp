// Internal
#include "Viewer.hpp"
#include "WorkerThread.hpp"


namespace ORB_SLAM2 {

Viewer::Viewer() noexcept : mWorkerThread(std::make_unique<utilities::WorkerThread>([this]() { this->run(); })) {}

Viewer::~Viewer() noexcept = default;

// Main thread function. Draw points, keyframes, the current camera pose and the last processed
// frame. Drawing is refreshed according to the camera fps. We use Pangolin.
void Viewer::run() noexcept {
  mbFinished = false;
  mbStopped  = false;

  initialize();
  while(mbFinishRequested) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    preDraw();
    draw();
    afterDraw();
  }
  cleanup();
}

bool Viewer::stop() noexcept {
  if(mbFinishRequested) {
    return false;
  }

  if(mbStopRequested) {
    mbStopped       = true;
    mbStopRequested = false;
    return true;
  }

  return false;
}

void Viewer::requestStop() noexcept {
  if(!mbStopped) {
    mbStopRequested = true;
  }
}

bool Viewer::isStopped() const noexcept {
  return mbStopped;
}

void Viewer::requestFinish() noexcept {
  mbFinishRequested = true;
}

bool Viewer::checkFinish() const noexcept {
  return mbFinishRequested;
}

} // namespace ORB_SLAM2

