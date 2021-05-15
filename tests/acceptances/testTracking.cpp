// STL
#include <cstdlib>
// OpenCV2
#include <opencv2/opencv.hpp>
// Internal
#include "System.hpp"
#include "Tracking.hpp"


int main(int argc, char* argv[]) {
  cv::Mat image = cv::imread("test.png", cv::IMREAD_COLOR);

  auto pTracking = ORB_SLAM2::Tracking::create(
      nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr,
      "EuRoC.yaml", ORB_SLAM2::System::eSensor::MONOCULAR);
  if(!pTracking) {
    return EXIT_FAILURE;
  }

  pTracking->GrabImageMonocular(image, 0);

  return EXIT_SUCCESS;
}

