// Internal
#include "Frame.hpp"
#include "ORBextractor.hpp"
#include "ORBVocabulary.hpp"


int main(int argc, char* argv[]) {
    const cv::Mat image = cv::imread("", cv::IMREAD_COLOR);
    const cv::Mat K;
    const cv::Mat distCoef;
    const double timeStamp = 0;
    ORB_SLAM2::ORBextractor extractor;
    ORB_SLAM2::RBVocabulary vocabulary;

    ORB_SLAM2::Frame frame(image, timeStamp,
                           &extractor, &vocabulary,
                           K, distCoef, bf, thDepth)

    return EXIT_SUCCESS;
}