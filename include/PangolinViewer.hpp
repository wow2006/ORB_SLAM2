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
#include "Viewer.hpp"

namespace ORB_SLAM2 {

class MapDrawer;

class PangolinViewer final : public Viewer {
public:
  explicit PangolinViewer(MapDrawer *pMapDrawer) noexcept;

  ~PangolinViewer() noexcept override;

  // TODO(Hussein): Find a way to make sure that every drived class register into this factory
  static std::shared_ptr<Viewer> createUsingSettings(MapDrawer *pMapDrawer, std::string_view settingsPath);

private:
  void initialize() noexcept override;

  void preDraw() noexcept override;

  void draw() noexcept override;

  void afterDraw() noexcept override;

  void cleanup() noexcept override;

  MapDrawer *mpMapDrawer;

  // 1/fps in ms
  double mT;
  float mImageWidth, mImageHeight;

  float mViewpointX, mViewpointY, mViewpointZ, mViewpointF;

  pangolin::OpenGlMatrix mTwc;
  pangolin::OpenGlRenderState mCameraState;

  std::unique_ptr<pangolin::Var<bool>> m_pMenuFollowCamera;
  std::unique_ptr<pangolin::Var<bool>> m_pMenuShowPoints;
  std::unique_ptr<pangolin::Var<bool>> m_pMenuShowKeyFrames;
  std::unique_ptr<pangolin::Var<bool>> m_pMenuShowGraph;
  std::unique_ptr<pangolin::Var<bool>> m_pMenuLocalizationMode;
  std::unique_ptr<pangolin::Var<bool>> m_pMenuReset;

  bool bFollow = true;
  bool bLocalizationMode = false;

};

}  // namespace ORB_SLAM2
