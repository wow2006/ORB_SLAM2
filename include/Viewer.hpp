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

namespace ORB_SLAM2 {

class System;
class Tracking;
class MapDrawer;
class FrameDrawer;

class Viewer final {
public:
  Viewer(System *pSystem, FrameDrawer *pFrameDrawer, MapDrawer *pMapDrawer, Tracking *pTracking, const std::string &strSettingPath);

  // Main thread function. Draw points, keyframes, the current camera pose and the last processed
  // frame. Drawing is refreshed according to the camera fps. We use Pangolin.
  void Run();

  void RequestFinish();

  void RequestStop();

  bool isFinished();

  bool isStopped();

  void Release();

private:
  bool Stop();

  System      *mpSystem      = nullptr;
  FrameDrawer *mpFrameDrawer = nullptr;
  MapDrawer   *mpMapDrawer   = nullptr;
  [[maybe_unused]] Tracking    *mpTracker = nullptr;

  // 1/fps in ms
  double mT = 0;
  float mImageWidth = 0;
  float mImageHeight = 0;

  float mViewpointX;
  float mViewpointY;
  float mViewpointZ;
  float mViewpointF;

  bool CheckFinish();

  void SetFinish();

  std::atomic_bool mbFinishRequested;
  std::atomic_bool mbFinished;

  std::atomic_bool mbStopped;
  std::atomic_bool mbStopRequested;

};

}  // namespace ORB_SLAM2
