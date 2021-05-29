// Internal
#include "ShowImageEvent.hpp"


static constexpr char g_sType[] = "ShowImageEvent";

ShowImageEvent::ShowImageEvent(cv::Mat imageToShow) noexcept : mImage{imageToShow} {}

ShowImageEvent::~ShowImageEvent() noexcept = default;

DescriptorType ShowImageEvent::type() const {
  return g_sType;
}

