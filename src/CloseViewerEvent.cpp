// Internal
#include "CloseViewerEvent.hpp"


CloseViewerEvent::~CloseViewerEvent() noexcept = default;

DescriptorType CloseViewerEvent::type() const {
  return "CloseViewerEvent";
}

