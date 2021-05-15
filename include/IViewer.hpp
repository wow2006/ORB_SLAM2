#pragma once

namespace ORB_SLAM2 {

struct IViewer {
  virtual ~IViewer() = default;

  virtual void draw() = 0;

};

} // namespace ORB_SLAM2

