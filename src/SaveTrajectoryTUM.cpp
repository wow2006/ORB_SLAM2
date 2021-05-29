// Internal
#include "SaveTrajectoryTUM.hpp"
//
#include "Map.hpp"
#include "System.hpp"
#include "Tracking.hpp"
#include "KeyFrame.hpp"
#include "Converter.hpp"


namespace ORB_SLAM2 {

SaveTrajectoryTUM::SaveTrajectoryTUM() noexcept = default;

SaveTrajectoryTUM::~SaveTrajectoryTUM() noexcept = default;

bool SaveTrajectoryTUM::save(std::ofstream &outputStream, const System &system) const {
  //spdlog::debug("Saving camera trajectory to {} ...", fileName);
  if(system.mSensor == System::MONOCULAR) {
    spdlog::error("ERROR: SaveTrajectoryTUM cannot be used for monocular.");
    return false;
  }

  auto vpKFs = system.mpMap->GetAllKeyFrames();
  std::sort(vpKFs.begin(), vpKFs.end(), KeyFrame::lId);

  // Transform all keyframes so that the first keyframe is at the origin.
  // After a loop closure the first keyframe might not be at the origin.
  const cv::Mat tWO = vpKFs[0]->GetPoseInverse();

  /*
  std::ofstream f(fileName);
  if(f.is_open()) {
    return false;
  }
  */
  outputStream << fixed;

  // Frame pose is stored relative to its reference keyframe (which is optimized by BA and pose graph).
  // We need to get first the keyframe pose and then concatenate the relative transformation.
  // Frames not localized (tracking failure) are not saved.

  // For each frame we have a reference keyframe (lRit), the timestamp (lT) and a flag
  // which is true when tracking failed (lbL).
  auto lRit = system.mpTracker->mlpReferences.begin();
  auto lT   = system.mpTracker->mlFrameTimes.begin();
  auto lbL  = system.mpTracker->mlbLost.begin();
  for(auto lit = system.mpTracker->mlRelativeFramePoses.begin(),
      lend = system.mpTracker->mlRelativeFramePoses.end(); lit != lend;
      lit++, lRit++, lT++, lbL++) {
    if(*lbL) {
      continue;
    }

    KeyFrame *pKF = *lRit;

    cv::Mat tRW = cv::Mat::eye(4, 4, CV_32F);

    // If the reference keyframe was culled, traverse the spanning tree to get a suitable keyframe.
    while(pKF->isBad()) {
      tRW = tRW * pKF->mTcp;
      pKF = pKF->GetParent();
    }

    tRW = tRW * pKF->GetPose() * tWO;

    cv::Mat tCW = (*lit) * tRW;
    cv::Mat rWC = tCW.rowRange(0, 3).colRange(0, 3).t();
    cv::Mat twc = -rWC * tCW.rowRange(0, 3).col(3);

    std::vector<float> q = Converter::toQuaternion(rWC);

    outputStream << setprecision(6) << *lT << " " << setprecision(9) << twc.at<float>(0) << " " << twc.at<float>(1) << " "
      << twc.at<float>(2) << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << endl;
  }
  //f.close();
  spdlog::debug("trajectory saved!");
  return true;
}

bool SaveTrajectoryTUM::saveKeyFrame(std::ofstream &outputStream, const System &system) const {
  //spdlog::debug("Saving keyframe trajectory to {} ...", filename);

  auto vpKFs = system.mpMap->GetAllKeyFrames();
  std::sort(vpKFs.begin(), vpKFs.end(), KeyFrame::lId);

  // Transform all keyframes so that the first keyframe is at the origin.
  // After a loop closure the first keyframe might not be at the origin.
  //cv::Mat Two = vpKFs[0]->GetPoseInverse();

  //std::ofstream f;
  //f.open(filename.c_str());
  outputStream << fixed;

  for(auto& pKF : vpKFs) {
    // pKF->SetPose(pKF->GetPose()*Two);
    if(pKF->isBad()) {
      continue;
    }

    const cv::Mat R = pKF->GetRotation().t();
    std::vector<float> q = Converter::toQuaternion(R);
    cv::Mat t = pKF->GetCameraCenter();
    outputStream << setprecision(6) << pKF->mTimeStamp << setprecision(7) << " " << t.at<float>(0) << " " << t.at<float>(1) << " "
                 << t.at<float>(2) << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << endl;
  }
  //f.close();
  spdlog::debug("trajectory saved!");

  return true;
}

} // namespace ORB_SLAM2
