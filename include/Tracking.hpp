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
// Internal
#include "Frame.hpp"
#include "ORBVocabulary.hpp"

namespace ORB_SLAM2 {

class Map;
class Viewer;
class System;
class MapDrawer;
class Initializer;
class FrameDrawer;
class LoopClosing;
class LocalMapping;
class KeyFrameDatabase;

class Tracking final {
public:
  static std::shared_ptr<Tracking>
  create(System *pSys, ORBVocabulary *pVoc, FrameDrawer *pFrameDrawer,
         MapDrawer *pMapDrawer, Map *pMap, KeyFrameDatabase *pKFDB,
         const std::string &strSettingPath, int sensor);

  Tracking(System *pSys, ORBVocabulary *pVoc, FrameDrawer *pFrameDrawer, MapDrawer *pMapDrawer, Map *pMap, KeyFrameDatabase *pKFDB, int sensor);

  // Preprocess the input and call Track(). Extract features and performs stereo matching.
  cv::Mat GrabImageStereo(const cv::Mat &imRectLeft, const cv::Mat &imRectRight, const double &timestamp);

  cv::Mat GrabImageRGBD(const cv::Mat &imRGB, const cv::Mat &imD, const double &timestamp);

  cv::Mat GrabImageMonocular(const cv::Mat &im, const double &timestamp);

  void SetLocalMapper(LocalMapping *pLocalMapper);
  void SetLoopClosing(LoopClosing *pLoopClosing);
  void SetViewer(Viewer *pViewer);

  // Load new settings
  // The focal lenght should be similar or scale prediction will fail when projecting points
  // TODO: Modify MapPoint::PredictScale to take into account focal lenght
  [[maybe_unused]] void ChangeCalibration(const string &strSettingPath);

  // Use this function if you have deactivated local mapping and you only want to localize the camera.
  void InformOnlyTracking(const bool &flag);

public:
  // Tracking states
  enum eTrackingState { SYSTEM_NOT_READY = -1, NO_IMAGES_YET = 0, NOT_INITIALIZED = 1, OK = 2, LOST = 3 };

  eTrackingState mState = NO_IMAGES_YET;
  eTrackingState mLastProcessedState;

  // Input sensor
  int mSensor = 0;

  // Current Frame
  Frame mCurrentFrame;
  cv::Mat mImGray;

  // Initialization Variables (Monocular)
  std::vector<int> mvIniLastMatches;
  std::vector<int> mvIniMatches;
  std::vector<cv::Point2f> mvbPrevMatched;
  std::vector<cv::Point3f> mvIniP3D;
  Frame mInitialFrame;

  // Lists used to recover the full camera trajectory at the end of the execution.
  // Basically we store the reference keyframe for each frame and its relative transformation
  std::list<cv::Mat> mlRelativeFramePoses;
  std::list<KeyFrame *> mlpReferences;
  std::list<double> mlFrameTimes;
  std::list<bool> mlbLost;

  // True if local mapping is deactivated and we are performing only localization
  bool mbOnlyTracking = false;

  void Reset();

protected:
  // Main tracking function. It is independent of the input sensor.
  void Track();

  // Map initialization for stereo and RGB-D
  void StereoInitialization();

  // Map initialization for monocular
  void MonocularInitialization();
  void CreateInitialMapMonocular();

  void CheckReplacedInLastFrame();
  bool TrackReferenceKeyFrame();
  void UpdateLastFrame();
  bool TrackWithMotionModel();

  bool Relocalization();

  void UpdateLocalMap();
  void UpdateLocalPoints();
  void UpdateLocalKeyFrames();

  bool TrackLocalMap();
  void SearchLocalPoints();

  bool NeedNewKeyFrame();
  void CreateNewKeyFrame();

  // In case of performing only localization, this flag is true when there are no matches to
  // points in the map. Still tracking will continue if there are enough matches with temporal points.
  // In that case we are doing visual odometry. The system will try to do relocalization to recover
  // "zero-drift" localization to the map.
  bool mbVO = false;

  // Other Thread Pointers
  LocalMapping *mpLocalMapper = nullptr;
  LoopClosing *mpLoopClosing = nullptr;

  // ORB
  ORBextractor *mpORBextractorLeft = nullptr;
  ORBextractor *mpORBextractorRight = nullptr;
  ORBextractor *mpIniORBextractor = nullptr;

  // BoW
  ORBVocabulary *mpORBVocabulary = nullptr;
  KeyFrameDatabase *mpKeyFrameDB = nullptr;

  // Initalization (only for monocular)
  Initializer *mpInitializer = nullptr;

  //Local Map
  KeyFrame *mpReferenceKF = nullptr;
  std::vector<KeyFrame *> mvpLocalKeyFrames;
  std::vector<MapPoint *> mvpLocalMapPoints;

  // System
  System *mpSystem = nullptr;

  // Drawers
  Viewer      *mpViewer      = nullptr;
  FrameDrawer *mpFrameDrawer = nullptr;
  MapDrawer   *mpMapDrawer   = nullptr;

  // Map
  Map *mpMap = nullptr;

  // Calibration matrix
  cv::Mat mK;
  cv::Mat mDistCoef;
  float mbf = 0;

  // New KeyFrame rules (according to fps)
  int mMinFrames = 0;
  int mMaxFrames = 0;

  // Threshold close/far points
  // Points seen as close by the stereo/RGBD sensor are considered reliable
  // and inserted from just one frame. Far points requiere a match in two keyframes.
  float mThDepth = 0;

  // For RGB-D inputs only. For some datasets (e.g. TUM) the depthmap values are scaled.
  float mDepthMapFactor = 0;

  // Current matches in frame
  int mnMatchesInliers = 0;

  // Last Frame, KeyFrame and Relocalisation Info
  KeyFrame *mpLastKeyFrame = nullptr;
  Frame mLastFrame;
  unsigned int mnLastKeyFrameId = 0;
  unsigned int mnLastRelocFrameId = 0;

  // Motion Model
  cv::Mat mVelocity;

  // Color order (true RGB, false BGR, ignored if grayscale)
  bool mbRGB = false;

  std::list<MapPoint *> mlpTemporalPoints;

};

}  // namespace ORB_SLAM2

