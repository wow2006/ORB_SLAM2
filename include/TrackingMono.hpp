#pragma once


namespace ORB_SLAM2 {

class TrackingMono final {
public:
  ~TrackingMono();

private:
  void track();

  void initialization();

  void createInitialMap();

};

} // namespace ORB_SLAM2

