// Internal
#include "SaveTrajectoryKITTI.hpp"
//
#include "Map.hpp"
#include "System.hpp"
#include "Tracking.hpp"
#include "KeyFrame.hpp"
#include "Converter.hpp"

namespace ORB_SLAM2 {

SaveTrajectoryKITTI::SaveTrajectoryKITTI() noexcept = default;

SaveTrajectoryKITTI::~SaveTrajectoryKITTI() noexcept = default;

bool SaveTrajectoryKITTI::save(std::ofstream &outputStream, const System &system) const {
  //spdlog::debug("Saving camera trajectory to {} ...", fileName);
  if(system.mSensor == System::MONOCULAR) {
    spdlog::error("ERROR: SaveTrajectoryKITTI cannot be used for monocular.");
    return false;
  }

  std::vector<KeyFrame *> vpKFs = system.mpMap->GetAllKeyFrames();
  std::sort(vpKFs.begin(), vpKFs.end(), KeyFrame::lId);

  // Transform all keyframes so that the first keyframe is at the origin.
  // After a loop closure the first keyframe might not be at the origin.
  cv::Mat Two = vpKFs[0]->GetPoseInverse();

  outputStream << fixed;

  // Frame pose is stored relative to its reference keyframe (which is optimized by BA and pose graph).
  // We need to get first the keyframe pose and then concatenate the relative transformation.
  // Frames not localized (tracking failure) are not saved.

  // For each frame we have a reference keyframe (lRit), the timestamp (lT) and a flag
  // which is true when tracking failed (lbL).
  auto lRit = system.mpTracker->mlpReferences.begin();
  auto lT = system.mpTracker->mlFrameTimes.begin();
  for(auto lit = system.mpTracker->mlRelativeFramePoses.begin(), lend = system.mpTracker->mlRelativeFramePoses.end(); lit != lend;
      lit++, lRit++, lT++) {
    ORB_SLAM2::KeyFrame *pKF = *lRit;

    cv::Mat Trw = cv::Mat::eye(4, 4, CV_32F);

    while(pKF->isBad()) {
      Trw = Trw * pKF->mTcp;
      pKF = pKF->GetParent();
    }

    Trw = Trw * pKF->GetPose() * Two;

    cv::Mat Tcw = (*lit) * Trw;
    cv::Mat Rwc = Tcw.rowRange(0, 3).colRange(0, 3).t();
    cv::Mat twc = -Rwc * Tcw.rowRange(0, 3).col(3);

    outputStream << setprecision(9) << Rwc.at<float>(0, 0) << " " << Rwc.at<float>(0, 1) << " " << Rwc.at<float>(0, 2) << " "
                 << twc.at<float>(0) << " " << Rwc.at<float>(1, 0) << " " << Rwc.at<float>(1, 1) << " " << Rwc.at<float>(1, 2)
                 << " " << twc.at<float>(1) << " " << Rwc.at<float>(2, 0) << " " << Rwc.at<float>(2, 1) << " "
                 << Rwc.at<float>(2, 2) << " " << twc.at<float>(2) << endl;
  }
  //f.close();
  spdlog::debug("trajectory saved!");
  return true;
}

bool SaveTrajectoryKITTI::saveKeyFrame([[maybe_unused]] std::ofstream &outputStream,
                                       [[maybe_unused]] const System& system) const {
  return false;
}

}  // namespace ORB_SLAM2
