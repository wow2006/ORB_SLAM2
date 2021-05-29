// Internal
#include "WorkerThread.hpp"


namespace utilities {

WorkerThread::WorkerThread(std::function<void()> task) noexcept :
  mTask{std::move(task)}, mWorker([this]() { run(); }) {}

WorkerThread::~WorkerThread() noexcept {
  mRunning = false;
  mWorker.join();
}

void WorkerThread::run() {
  mRunning = true;
  while(mRunning) {
    mTask();
  }
}

} // namespace utilities

