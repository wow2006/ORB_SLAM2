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
#pragma once
// Interal
#include "ORBVocabulary.hpp"

class Dispatcher;
class ConfigurationSystem;

extern void usleep(uint32_t useconds);

// TODO(Hussein): Remove me
extern std::unique_ptr<Dispatcher> g_pDispatcher; // NOLINT
extern std::unique_ptr<ConfigurationSystem> g_pConfiguration; // NOLINT

namespace ORB_SLAM2 {

class Map;
struct Viewer;
class MapPoint;
class Tracking;
class MapDrawer;
class FrameDrawer;
class LoopClosing;
class LocalMapping;
class KeyFrameDatabase;

class System final {
public:
  // Input sensor
  enum eSensor { MONOCULAR = 0, STEREO = 1, RGBD = 2 };

  // Initialize the SLAM system. It launches the Local Mapping, Loop Closing and Viewer threads.
  System(const string &strVocFile, const string &strSettingsFile, eSensor sensor, bool bUseViewer = true);

  System(const System&) = delete;

  System& operator=(const System&) = delete;

  System(System&&) = delete;

  System& operator=(System&&) = delete;

  // Proccess the given stereo frame. Images must be synchronized and rectified.
  // Input images: RGB (CV_8UC3) or grayscale (CV_8U). RGB is converted to grayscale.
  // Returns the camera pose (empty if tracking fails).
  [[maybe_unused]] cv::Mat trackStereo(const cv::Mat &imLeft, const cv::Mat &imRight, const double &timestamp);

  // Process the given rgbd frame. Depthmap must be registered to the RGB frame.
  // Input image: RGB (CV_8UC3) or grayscale (CV_8U). RGB is converted to grayscale.
  // Input depthmap: Float (CV_32F).
  // Returns the camera pose (empty if tracking fails).
  [[maybe_unused]] cv::Mat trackRGBD(const cv::Mat &im, const cv::Mat &depthmap, const double &timestamp);

  // Proccess the given monocular frame
  // Input images: RGB (CV_8UC3) or grayscale (CV_8U). RGB is converted to grayscale.
  // Returns the camera pose (empty if tracking fails).
  [[maybe_unused]] cv::Mat trackMonocular(const cv::Mat &im, const double &timestamp);

  // This stops local mapping thread (map building) and performs only camera tracking.
  void ActivateLocalizationMode();

  // This resumes local mapping thread and performs SLAM again.
  void DeactivateLocalizationMode();

  // Returns true if there have been a big map change (loop closure, global BA)
  // since last call to this function
  [[maybe_unused]] bool MapChanged();

  // Reset the system (clear map)
  void Reset();

  // All threads will be requested to finish.
  // It waits until all threads have finished.
  // This function must be called before saving the trajectory.
  void Shutdown();

  enum class SaveFormat : uint8_t {
    TUM,
    KITTI
  };

  [[nodiscard]] bool save(std::string_view fileName, SaveFormat format) const;

  // TODO(Hussein): Save/Load functions
  // SaveMap(const string &filename);
  // LoadMap(const string &filename);

  // Information from most recent processed frame
  // You can call this right after TrackMonocular (or stereo or RGBD)
  [[maybe_unused]] int GetTrackingState();

  [[maybe_unused]] std::vector<MapPoint *> GetTrackedMapPoints();

  [[maybe_unused]] std::vector<cv::KeyPoint> GetTrackedKeyPointsUn();

private:
  // Input sensor
  eSensor mSensor;

  // ORB vocabulary used for place recognition and feature matching.
  ORBVocabulary *mpVocabulary;

  // KeyFrame database for place recognition (relocalization and loop detection).
  KeyFrameDatabase *mpKeyFrameDatabase;

  // Map structure that stores the pointers to all KeyFrames and MapPoints.
  Map *mpMap;

  // Tracker. It receives a frame and computes the associated camera pose.
  // It also decides when to insert a new keyframe, create some new MapPoints and
  // performs relocalization if tracking fails.
  Tracking *mpTracker;

  // Local Mapper. It manages the local map and performs local bundle adjustment.
  LocalMapping *mpLocalMapper;

  // Loop Closer. It searches loops with every new keyframe. If there is a loop it performs
  // a pose graph optimization and full bundle adjustment (in a new thread) afterwards.
  LoopClosing *mpLoopCloser;

  // The viewer draws the map and the current camera pose. It uses Pangolin.
  //std::shared_ptr<Viewer> m_pViewer;
  //FrameDrawer *mpFrameDrawer;
  //MapDrawer *mpMapDrawer;

  // System threads: Local Mapping, Loop Closing, Viewer.
  // The Tracking thread "lives" in the main execution thread that creates the System object.
  [[maybe_unused]] std::thread *mptLocalMapping;
  [[maybe_unused]] std::thread *mptLoopClosing;
  //[[maybe_unused]] std::thread *mptViewer;

  // Reset flag
  std::mutex mMutexReset;
  bool mbReset;

  // Change mode flags
  std::mutex mMutexMode;
  bool mbActivateLocalizationMode;
  bool mbDeactivateLocalizationMode;

  // Tracking state
  int mTrackingState = 0;
  std::vector<MapPoint *> mTrackedMapPoints;
  std::vector<cv::KeyPoint> mTrackedKeyPointsUn;
  std::mutex mMutexState;

  friend struct SaveTrajectoryTUM;
  friend struct SaveTrajectoryKITTI;

};

}  // namespace ORB_SLAM2
