#pragma once

namespace utilities {
class WorkerThread;
} // namespace utilities

namespace ORB_SLAM2 {

struct Viewer {
  Viewer() noexcept;

  Viewer(const Viewer&) noexcept = delete;

  Viewer& operator=(const Viewer&) noexcept = delete;

  Viewer(Viewer&&) noexcept = delete;

  Viewer& operator=(Viewer&&) noexcept = delete;

  virtual ~Viewer() noexcept;

  void requestStop() noexcept;

  [[nodiscard]] bool isStopped() const noexcept;

  void requestFinish() noexcept;

  [[nodiscard]] bool isFinished() const noexcept;

  void run() noexcept;

  bool stop() noexcept;

protected:
  [[nodiscard]] bool checkFinish() const noexcept;

private:
  virtual void initialize() noexcept = 0;

  virtual void preDraw() noexcept = 0;

  virtual void draw() noexcept = 0;

  virtual void afterDraw() noexcept = 0;

  virtual void cleanup() noexcept = 0;

  std::atomic_bool mbFinishRequested = false;
  std::atomic_bool mbFinished = true;

  std::atomic_bool mbStopped = true;
  std::atomic_bool mbStopRequested = false;

  std::unique_ptr<utilities::WorkerThread> mWorkerThread;

};

} // namespace ORB_SLAM2

