#pragma once
// Internal
#include "IEvent.hpp"


struct ShowImageEvent final : IEvent {
  explicit ShowImageEvent(cv::Mat imageToShow) noexcept;

  ~ShowImageEvent() noexcept override;

  DescriptorType type() const override;

  const cv::Mat get() const {
    return mImage;
  }

private:
  cv::Mat mImage;

};

