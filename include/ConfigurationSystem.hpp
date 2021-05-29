#pragma once

class ConfigurationSystem {
public:
  [[nodiscard]] int getViewerSleepDuration() const {
    return mViewerSleepDuration;
  }

private:
  int mViewerSleepDuration = 3000;

};

