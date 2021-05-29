#pragma once
// STL
#include <atomic>
#include <thread>


namespace utilities {

class WorkerThread final {
public:
  explicit WorkerThread(std::function<void()>) noexcept;

  WorkerThread(const WorkerThread&) = delete;

  WorkerThread& operator=(const WorkerThread&) = delete;

  // NOTE(Hussein): I think move is ok
  WorkerThread(WorkerThread&&) = delete;

  WorkerThread& operator=(WorkerThread&&) = delete;

  ~WorkerThread() noexcept;

  void stop() noexcept {
    mRunning = false;
  }

private:
  void run();

  std::atomic_bool mRunning = false;

  std::function<void()> mTask;

  std::thread mWorker;

};

} // namespace utilities

