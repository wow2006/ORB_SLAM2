#pragma once
// STL
#include <string>


using DescriptorType = std::string;

struct IEvent {
  virtual ~IEvent() = default;

  virtual DescriptorType type() const = 0;

};

