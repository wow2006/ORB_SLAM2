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
#include "Dispatcher.hpp"
#include "FrameDrawer.hpp"
// Save
#include "ISaveTrajectory.hpp"
#include "SaveTrajectoryTUM.hpp"
#include "SaveTrajectoryKITTI.hpp"
//
#include "LoopClosing.hpp"
#include "LocalMapping.hpp"
#include "ShowImageEvent.hpp"
#include "PangolinViewer.hpp"
#include "KeyFrameDatabase.hpp"
#include "CloseViewerEvent.hpp"
#include "ConfigurationSystem.hpp"


void usleep(uint32_t useconds) { std::this_thread::sleep_for(std::chrono::microseconds(useconds)); }

std::unique_ptr<Dispatcher> g_pDispatcher; // NOLINT
std::unique_ptr<ConfigurationSystem> g_pConfiguration; // NOLINT

inline void createImageShowEvent(std::shared_ptr<ORB_SLAM2::Viewer> pViewer) {
  ShowImageEvent showImageEvent({});
  const auto key = showImageEvent.type();

  g_pDispatcher->subscribe(key, [pViewer=std::move(pViewer)](const IEvent& event) {
    const auto* showImageEventCasted = dynamic_cast<const ShowImageEvent*>(&event);
    const auto image = showImageEventCasted->get();
    cv::imshow("ORB-SLAM2: Current Frame", image);
  });
}

inline void createCloseViewerEvent(std::shared_ptr<ORB_SLAM2::Viewer> pViewer) {
  CloseViewerEvent closeEvent;
  const auto key = closeEvent.type();

  g_pDispatcher->subscribe(key, [pViewer](const IEvent&) {
    pViewer->requestStop();
    while(!pViewer->isStopped()) {
      usleep(g_pConfiguration->getViewerSleepDuration());
    }
  });
}

namespace ORB_SLAM2 {

System::System(const string &strVocFile, const string &strSettingsFile,
              const eSensor sensor, const bool bUseViewer)
  : mSensor(sensor), mbReset(false), mbActivateLocalizationMode(false),
    mbDeactivateLocalizationMode(false) {
  if(!g_pDispatcher) {
    g_pDispatcher = std::make_unique<Dispatcher>();
  }
  if(!g_pConfiguration) {
    g_pConfiguration = std::make_unique<ConfigurationSystem>();
  }

  // TODO(Hussein): Move this to other location
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
  //mpFrameDrawer = new FrameDrawer(mpMap);
  //mpMapDrawer = new MapDrawer(mpMap, strSettingsFile);

  //Initialize the Tracking thread
  //(it will live in the main thread of execution, the one that called this constructor)
  mpTracker = new Tracking(this, mpVocabulary, nullptr/*mpFrameDrawer*/, nullptr/*mpMapDrawer*/, mpMap, mpKeyFrameDatabase, strSettingsFile, mSensor);

  //Initialize the Local Mapping thread and launch
  mpLocalMapper = new LocalMapping(mpMap, mSensor == MONOCULAR);
  mptLocalMapping = new thread(&ORB_SLAM2::LocalMapping::Run, mpLocalMapper);

  //Initialize the Loop Closing thread and launch
  mpLoopCloser = new LoopClosing(mpMap, mpKeyFrameDatabase, mpVocabulary, mSensor != MONOCULAR);
  mptLoopClosing = new thread(&ORB_SLAM2::LoopClosing::Run, mpLoopCloser);

  //Initialize the Viewer thread and launch
  /*
  if(bUseViewer) {
    m_pViewer = PangolinViewer::createUsingSettings(mpMapDrawer, strSettingsFile);
    mptViewer = new thread(&PangolinViewer::Run, m_pViewer.get()); // TODO(Hussein): Move this to Viewer
    createImageShowEvent(m_pViewer);
  }
  */

  //Set pointers between threads
  mpTracker->SetLocalMapper(mpLocalMapper);
  mpTracker->SetLoopClosing(mpLoopCloser);

  mpLocalMapper->SetTracker(mpTracker);
  mpLocalMapper->SetLoopCloser(mpLoopCloser);

  mpLoopCloser->SetTracker(mpTracker);
  mpLoopCloser->SetLocalMapper(mpLocalMapper);
}

cv::Mat System::trackStereo(const cv::Mat &imLeft, const cv::Mat &imRight, const double &timestamp) {
  if(mSensor != STEREO) {
    spdlog::error("ERROR: you called TrackStereo but input sensor was not set to STEREO.");
    std::exit(-1); // TODO(Hussein): Remove this
  }

  static uint32_t index = 0;
  spdlog::debug("TrackStereo: {}", index++);

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

  cv::Mat tCW = mpTracker->GrabImageStereo(imLeft, imRight, timestamp);

  std::unique_lock<std::mutex> lock2(mMutexState);
  mTrackingState      = static_cast<int>(mpTracker->mState); // TODO(Hussein): Remove convertion
  mTrackedMapPoints   = mpTracker->mCurrentFrame.mvpMapPoints;
  mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;

  return tCW;
}

cv::Mat System::trackRGBD(const cv::Mat &im, const cv::Mat &depthmap, const double &timestamp) {
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

cv::Mat System::trackMonocular(const cv::Mat &im, const double &timestamp) {
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
  /*
  if(m_pViewer) {
    m_pViewer->requestFinish();
    while(!m_pViewer->isFinished()) {
      usleep(5000);
    }
  }
  */

  // Wait until all thread have effectively stopped
  while(!mpLocalMapper->isFinished() || !mpLoopCloser->isFinished() || mpLoopCloser->isRunningGBA()) {
    usleep(5000);
  }

  /*
  if(m_pViewer) {
    pangolin::BindToContext("ORB-SLAM2: Map Viewer");
  }
  */
}

bool System::save(std::string_view fileName, SaveFormat format) const {
  ISaveTrajectory* pSaver = nullptr;
  if(format == SaveFormat::TUM) {
    pSaver = new SaveTrajectoryTUM;
  } else if(format == SaveFormat::KITTI) {
    pSaver = new SaveTrajectoryKITTI;
  }

  std::ofstream saveTrajectory(fmt::format("{}.txt", fileName.data()));
  if(pSaver->save(saveTrajectory, *this)) {
  }
  std::ofstream saveKeyFrameTrajectory(fmt::format("{}_keyframe.txt", fileName.data()));
  if(pSaver->saveKeyFrame(saveKeyFrameTrajectory, *this)) {
  }
  return true;
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
