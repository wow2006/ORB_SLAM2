/**
 * This file is part of ORB-SLAM2.
 *
 * Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
 * For more information see <https://github.com/raulmur/ORB_SLAM2>
 *
 * ORB-SLAM2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ORB-SLAM2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
 */
#include "System.hpp"
// Internal
#include "Map.hpp"
#include "Viewer.hpp"
#include "Tracking.hpp"
#include "KeyFrame.hpp"
#include "MapDrawer.hpp"
#include "Converter.hpp"
#include "FrameDrawer.hpp"
#include "LoopClosing.hpp"
#include "LocalMapping.hpp"
#include "ShowImageEvent.hpp"
#include "KeyFrameDatabase.hpp"


void usleep(uint32_t useconds) { std::this_thread::sleep_for(std::chrono::microseconds(useconds)); }

std::shared_ptr<Dispatcher> g_pDispatcher; // NOLINT

inline void createImageShowEvent(std::shared_ptr<ORB_SLAM2::IViewer> pViewer) {
  ShowImageEvent showImageEvent({});
  const auto key = showImageEvent.type();

  g_pDispatcher->subscribe(key, [pViewer](const IEvent& event) {
    const auto showImageEventCasted = static_cast<const ShowImageEvent*>(&event);
    const auto image = showImageEventCasted->get();
    cv::imshow("ORB-SLAM2: Current Frame", image);
  });
}

namespace ORB_SLAM2 {

System::System(const string &strVocFile, const string &strSettingsFile,
              const eSensor sensor, const bool bUseViewer)
  : mSensor(sensor), mbReset(false), mbActivateLocalizationMode(false),
    mbDeactivateLocalizationMode(false) {
  if(!g_pDispatcher) {
    g_pDispatcher = std::make_shared<Dispatcher>();
  }
  // Output welcome message
  spdlog::debug("ORB-SLAM2 Copyright (C) 2014-2016 Raul Mur-Artal, University of Zaragoza.");
  spdlog::debug("This program comes with ABSOLUTELY NO WARRANTY;");
  spdlog::debug("This is free software, and you are welcome to redistribute it");
  spdlog::debug("under certain conditions. See LICENSE.txt.");
  spdlog::debug("");

  spdlog::debug("Input sensor was set to: ");

  if(mSensor == MONOCULAR) {
    spdlog::debug("Monocular");
  } else if(mSensor == STEREO) {
    spdlog::debug("Stereo");
  } else if(mSensor == RGBD) {
    spdlog::debug("RGB-D");
  }

  //Check settings file
  cv::FileStorage fsSettings(strSettingsFile.c_str(), cv::FileStorage::READ);
  if(!fsSettings.isOpened()) {
    spdlog::error("Failed to open settings file at: {}", strSettingsFile);
    exit(-1); // TODO(Hussein): Remove this
  }

  //Load ORB Vocabulary
  spdlog::debug("Loading ORB Vocabulary. This could take a while...");

  mpVocabulary = new ORBVocabulary();

  bool bVocLoad = false;
  auto found = strVocFile.rfind(".txt");
  if(found) {
    bVocLoad = mpVocabulary->loadFromTextFile(strVocFile);
  } else {
    bVocLoad = mpVocabulary->loadFromBinFile(strVocFile);
  }

  if(!bVocLoad) {
    spdlog::error("Wrong path to vocabulary. ");
    spdlog::error("Falied to open at: {}", strVocFile);
    exit(-1); // TODO(Hussein): Remove this
  }
  spdlog::debug("Vocabulary loaded!");

  //Create KeyFrame Database
  mpKeyFrameDatabase = new KeyFrameDatabase(*mpVocabulary);

  //Create the Map
  mpMap = new Map();

  //Create Drawers. These are used by the Viewer
  mpFrameDrawer = new FrameDrawer(mpMap);
  mpMapDrawer = new MapDrawer(mpMap, strSettingsFile);

  //Initialize the Tracking thread
  //(it will live in the main thread of execution, the one that called this constructor)
  mpTracker = new Tracking(this, mpVocabulary, mpFrameDrawer, mpMapDrawer, mpMap, mpKeyFrameDatabase, strSettingsFile, mSensor);

  //Initialize the Local Mapping thread and launch
  mpLocalMapper = new LocalMapping(mpMap, mSensor == MONOCULAR);
  mptLocalMapping = new thread(&ORB_SLAM2::LocalMapping::Run, mpLocalMapper);

  //Initialize the Loop Closing thread and launch
  mpLoopCloser = new LoopClosing(mpMap, mpKeyFrameDatabase, mpVocabulary, mSensor != MONOCULAR);
  mptLoopClosing = new thread(&ORB_SLAM2::LoopClosing::Run, mpLoopCloser);

  //Initialize the Viewer thread and launch
  if(bUseViewer) {
    m_pViewer = Viewer::createUsingSettings(mpMapDrawer, strSettingsFile);
    mptViewer = new thread(&Viewer::Run, m_pViewer.get());
    //createImageShowEvent(m_pViewer);
  }

  //Set pointers between threads
  mpTracker->SetLocalMapper(mpLocalMapper);
  mpTracker->SetLoopClosing(mpLoopCloser);

  mpLocalMapper->SetTracker(mpTracker);
  mpLocalMapper->SetLoopCloser(mpLoopCloser);

  mpLoopCloser->SetTracker(mpTracker);
  mpLoopCloser->SetLocalMapper(mpLocalMapper);
}

[[maybe_unused]] cv::Mat System::TrackStereo(const cv::Mat &imLeft, const cv::Mat &imRight, const double &timestamp) {
  if(mSensor != STEREO) {
    spdlog::error("ERROR: you called TrackStereo but input sensor was not set to STEREO.");
    std::exit(-1); // TODO(Hussein): Remove this
  }

  // Check mode change
  {
    std::unique_lock<std::mutex> lock(mMutexMode);
    if(mbActivateLocalizationMode) {
      mpLocalMapper->RequestStop();

      // Wait until Local Mapping has effectively stopped
      while(!mpLocalMapper->isStopped()) {
        usleep(1000);
      }

      mpTracker->InformOnlyTracking(true);
      mbActivateLocalizationMode = false;
    }

    if(mbDeactivateLocalizationMode) {
      mpTracker->InformOnlyTracking(false);
      mpLocalMapper->Release();
      mbDeactivateLocalizationMode = false;
    }
  }

  // Check reset
  {
    std::unique_lock<std::mutex> lock(mMutexReset);
    if(mbReset) {
      mpTracker->Reset();
      mbReset = false;
    }
  }

  const cv::Mat Tcw = mpTracker->GrabImageStereo(imLeft, imRight, timestamp);

  std::unique_lock<std::mutex> lock2(mMutexState);
  mTrackingState      = static_cast<int>(mpTracker->mState); // TODO(Hussein): Remove convertion
  mTrackedMapPoints   = mpTracker->mCurrentFrame.mvpMapPoints;
  mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;

  return Tcw;
}

[[maybe_unused]] cv::Mat System::TrackRGBD(const cv::Mat &im, const cv::Mat &depthmap, const double &timestamp) {
  if(mSensor != RGBD) {
    spdlog::error("ERROR: you called TrackRGBD but input sensor was not set to RGBD.");
    std::exit(-1); // TODO(Hussein): Remove this
  }

  // Check mode change
  {
    std::unique_lock<std::mutex> lock(mMutexMode);
    if(mbActivateLocalizationMode) {
      mpLocalMapper->RequestStop();

      // Wait until Local Mapping has effectively stopped
      while(!mpLocalMapper->isStopped()) {
        usleep(1000);
      }

      mpTracker->InformOnlyTracking(true);
      mbActivateLocalizationMode = false;
    }

    if(mbDeactivateLocalizationMode) {
      mpTracker->InformOnlyTracking(false);
      mpLocalMapper->Release();
      mbDeactivateLocalizationMode = false;
    }
  }

  // Check reset
  {
    std::unique_lock<std::mutex> lock(mMutexReset);
    if(mbReset) {
      mpTracker->Reset();
      mbReset = false;
    }
  }

  cv::Mat Tcw = mpTracker->GrabImageRGBD(im, depthmap, timestamp);

  std::unique_lock<std::mutex> lock2(mMutexState);
  mTrackingState = static_cast<int>(mpTracker->mState); // TODO(Hussein): Remove casting
  mTrackedMapPoints = mpTracker->mCurrentFrame.mvpMapPoints;
  mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;

  return Tcw;
}

[[maybe_unused]] cv::Mat System::TrackMonocular(const cv::Mat &im, const double &timestamp) {
  if(mSensor != MONOCULAR) {
    spdlog::error("ERROR: you called TrackMonocular but input sensor was not set to Monocular.");
    exit(-1); // TODO(Hussein): Remove this
  }

  // Check mode change
  {
    std::unique_lock<std::mutex> lock(mMutexMode);
    if(mbActivateLocalizationMode) {
      mpLocalMapper->RequestStop();

      // Wait until Local Mapping has effectively stopped
      while(!mpLocalMapper->isStopped()) {
        usleep(1000);
      }

      mpTracker->InformOnlyTracking(true);
      mbActivateLocalizationMode = false;
    }

    if(mbDeactivateLocalizationMode) {
      mpTracker->InformOnlyTracking(false);
      mpLocalMapper->Release();
      mbDeactivateLocalizationMode = false;
    }
  }

  // Check reset
  {
    std::unique_lock<std::mutex> lock(mMutexReset);
    if(mbReset) {
      mpTracker->Reset();
      mbReset = false;
    }
  }

  cv::Mat Tcw = mpTracker->GrabImageMonocular(im, timestamp);

  std::unique_lock<std::mutex> lock2(mMutexState);
  mTrackingState = static_cast<int>(mpTracker->mState); // TODO(Hussein): Remove casting
  mTrackedMapPoints = mpTracker->mCurrentFrame.mvpMapPoints;
  mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;

  return Tcw;
}

void System::ActivateLocalizationMode() {
  std::unique_lock<std::mutex> lock(mMutexMode);
  mbActivateLocalizationMode = true;
}

void System::DeactivateLocalizationMode() {
  std::unique_lock<std::mutex> lock(mMutexMode);
  mbDeactivateLocalizationMode = true;
}

[[maybe_unused]] bool System::MapChanged() {
  static int n = 0;
  int curn = mpMap->GetLastBigChangeIdx();
  if(n < curn) {
    n = curn;
    return true;
  }
  return false;
}

void System::Reset() {
  std::unique_lock<std::mutex> lock(mMutexReset);
  mbReset = true;
}

void System::Shutdown() {
  mpLocalMapper->RequestFinish();
  mpLoopCloser->RequestFinish();
  if(m_pViewer) {
    m_pViewer->RequestFinish();
    while(!m_pViewer->isFinished()) {
      usleep(5000);
    }
  }

  // Wait until all thread have effectively stopped
  while(!mpLocalMapper->isFinished() || !mpLoopCloser->isFinished() || mpLoopCloser->isRunningGBA()) {
    usleep(5000);
  }

  if(m_pViewer) {
    pangolin::BindToContext("ORB-SLAM2: Map Viewer");
  }
}

[[maybe_unused]] void System::SaveTrajectoryTUM(const string &filename) {
  spdlog::debug("Saving camera trajectory to {} ...", filename);
  if(mSensor == MONOCULAR) {
    spdlog::error("ERROR: SaveTrajectoryTUM cannot be used for monocular.");
    return;
  }

  std::vector<KeyFrame *> vpKFs = mpMap->GetAllKeyFrames();
  std::sort(vpKFs.begin(), vpKFs.end(), KeyFrame::lId);

  // Transform all keyframes so that the first keyframe is at the origin.
  // After a loop closure the first keyframe might not be at the origin.
  cv::Mat Two = vpKFs[0]->GetPoseInverse();

  std::ofstream f;
  f.open(filename.c_str());
  f << fixed;

  // Frame pose is stored relative to its reference keyframe (which is optimized by BA and pose graph).
  // We need to get first the keyframe pose and then concatenate the relative transformation.
  // Frames not localized (tracking failure) are not saved.

  // For each frame we have a reference keyframe (lRit), the timestamp (lT) and a flag
  // which is true when tracking failed (lbL).
  auto lRit = mpTracker->mlpReferences.begin();
  auto lT   = mpTracker->mlFrameTimes.begin();
  auto lbL  = mpTracker->mlbLost.begin();
  for(auto lit = mpTracker->mlRelativeFramePoses.begin(), lend = mpTracker->mlRelativeFramePoses.end(); lit != lend;
      lit++, lRit++, lT++, lbL++) {
    if(*lbL) {
      continue;
    }

    KeyFrame *pKF = *lRit;

    cv::Mat Trw = cv::Mat::eye(4, 4, CV_32F);

    // If the reference keyframe was culled, traverse the spanning tree to get a suitable keyframe.
    while(pKF->isBad()) {
      Trw = Trw * pKF->mTcp;
      pKF = pKF->GetParent();
    }

    Trw = Trw * pKF->GetPose() * Two;

    cv::Mat Tcw = (*lit) * Trw;
    cv::Mat Rwc = Tcw.rowRange(0, 3).colRange(0, 3).t();
    cv::Mat twc = -Rwc * Tcw.rowRange(0, 3).col(3);

    std::vector<float> q = Converter::toQuaternion(Rwc);

    f << setprecision(6) << *lT << " " << setprecision(9) << twc.at<float>(0) << " " << twc.at<float>(1) << " "
      << twc.at<float>(2) << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << endl;
  }
  f.close();
  spdlog::debug("trajectory saved!");
}

[[maybe_unused]] void System::SaveKeyFrameTrajectoryTUM(const string &filename) {
  spdlog::debug("Saving keyframe trajectory to {} ...", filename);

  auto vpKFs = mpMap->GetAllKeyFrames();
  std::sort(vpKFs.begin(), vpKFs.end(), KeyFrame::lId);

  // Transform all keyframes so that the first keyframe is at the origin.
  // After a loop closure the first keyframe might not be at the origin.
  //cv::Mat Two = vpKFs[0]->GetPoseInverse();

  std::ofstream f;
  f.open(filename.c_str());
  f << fixed;

  for(auto pKF : vpKFs) {
    // pKF->SetPose(pKF->GetPose()*Two);

    if(pKF->isBad()) {
      continue;
    }

    cv::Mat R = pKF->GetRotation().t();
    std::vector<float> q = Converter::toQuaternion(R);
    cv::Mat t = pKF->GetCameraCenter();
    f << setprecision(6) << pKF->mTimeStamp << setprecision(7) << " " << t.at<float>(0) << " " << t.at<float>(1) << " "
      << t.at<float>(2) << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << endl;
  }

  f.close();
  spdlog::debug("trajectory saved!");
}

[[maybe_unused]] void System::SaveTrajectoryKITTI(const string &filename) {
  spdlog::debug("Saving camera trajectory to {} ...", filename);
  if(mSensor == MONOCULAR) {
    spdlog::error("ERROR: SaveTrajectoryKITTI cannot be used for monocular.");
    return;
  }

  std::vector<KeyFrame *> vpKFs = mpMap->GetAllKeyFrames();
  std::sort(vpKFs.begin(), vpKFs.end(), KeyFrame::lId);

  // Transform all keyframes so that the first keyframe is at the origin.
  // After a loop closure the first keyframe might not be at the origin.
  cv::Mat Two = vpKFs[0]->GetPoseInverse();

  ofstream f;
  f.open(filename.c_str());
  f << fixed;

  // Frame pose is stored relative to its reference keyframe (which is optimized by BA and pose graph).
  // We need to get first the keyframe pose and then concatenate the relative transformation.
  // Frames not localized (tracking failure) are not saved.

  // For each frame we have a reference keyframe (lRit), the timestamp (lT) and a flag
  // which is true when tracking failed (lbL).
  auto lRit = mpTracker->mlpReferences.begin();
  auto lT   = mpTracker->mlFrameTimes.begin();
  for(auto lit = mpTracker->mlRelativeFramePoses.begin(), lend = mpTracker->mlRelativeFramePoses.end(); lit != lend;
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

    f << setprecision(9) << Rwc.at<float>(0, 0) << " " << Rwc.at<float>(0, 1) << " " << Rwc.at<float>(0, 2) << " " << twc.at<float>(0)
      << " " << Rwc.at<float>(1, 0) << " " << Rwc.at<float>(1, 1) << " " << Rwc.at<float>(1, 2) << " " << twc.at<float>(1) << " "
      << Rwc.at<float>(2, 0) << " " << Rwc.at<float>(2, 1) << " " << Rwc.at<float>(2, 2) << " " << twc.at<float>(2) << endl;
  }
  f.close();
  spdlog::debug("trajectory saved!");
}

int System::GetTrackingState() {
  std::unique_lock<std::mutex> lock(mMutexState);
  return mTrackingState;
}

std::vector<MapPoint *> System::GetTrackedMapPoints() {
  std::unique_lock<std::mutex> lock(mMutexState);
  return mTrackedMapPoints;
}

std::vector<cv::KeyPoint> System::GetTrackedKeyPointsUn() {
  std::unique_lock<std::mutex> lock(mMutexState);
  return mTrackedKeyPointsUn;
}

}  // namespace ORB_SLAM2
