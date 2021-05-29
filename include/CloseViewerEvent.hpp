#pragma once
// Internal
#include "IEvent.hpp"


struct CloseViewerEvent final : IEvent {
  explicit CloseViewerEvent() noexcept = default;

  CloseViewerEvent(const CloseViewerEvent&) noexcept = default;

  CloseViewerEvent& operator=(const CloseViewerEvent&) noexcept = default;

  CloseViewerEvent(CloseViewerEvent&&) noexcept = default;

  CloseViewerEvent& operator=(CloseViewerEvent&&) noexcept = default;

  ~CloseViewerEvent() noexcept override;

  [[nodiscard]] DescriptorType type() const override;

};

