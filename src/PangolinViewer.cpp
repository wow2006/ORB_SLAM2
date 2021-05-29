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
// Internal
#include "PangolinViewer.hpp"
//
#include "System.hpp"
#include "Tracking.hpp"
#include "MapDrawer.hpp"
#include "FrameDrawer.hpp"
// pangolin
#include <pangolin/pangolin.h>

namespace ORB_SLAM2 {

PangolinViewer::PangolinViewer(MapDrawer *pMapDrawer) noexcept : mpMapDrawer(pMapDrawer) {}

PangolinViewer::~PangolinViewer() noexcept = default;

std::shared_ptr<Viewer> PangolinViewer::createUsingSettings(MapDrawer *pMapDrawer, std::string_view settingsPath) {
  cv::FileStorage fSettings(settingsPath.data(), cv::FileStorage::READ);
  if(!fSettings.isOpened()) {
    spdlog::error("ERROR: Can not");
    return nullptr;
  }

  auto pPangolinViewer = std::make_shared<PangolinViewer>(pMapDrawer);
  float fps = fSettings["Camera.fps"];
  if(fps < 1) {
    fps = 30;
  }
  pPangolinViewer->mT = 1e3 / fps;

  pPangolinViewer->mImageWidth = fSettings["Camera.width"];
  pPangolinViewer->mImageHeight = fSettings["Camera.height"];
  if(pPangolinViewer->mImageWidth < 1 || pPangolinViewer->mImageHeight < 1) {
    pPangolinViewer->mImageWidth = 640;
    pPangolinViewer->mImageHeight = 480;
  }

  pPangolinViewer->mViewpointX = fSettings["PangolinViewer.ViewpointX"];
  pPangolinViewer->mViewpointY = fSettings["PangolinViewer.ViewpointY"];
  pPangolinViewer->mViewpointZ = fSettings["PangolinViewer.ViewpointZ"];
  pPangolinViewer->mViewpointF = fSettings["PangolinViewer.ViewpointF"];

  return std::move(pPangolinViewer);
}

void PangolinViewer::initialize() noexcept {
  pangolin::CreateWindowAndBind("ORB-SLAM2: Map PangolinViewer", 1024, 768);

  // 3D Mouse handler requires depth testing to be enabled
  glEnable(GL_DEPTH_TEST);

  // Issue specific OpenGl we might need
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  pangolin::CreatePanel("menu").SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(175));
  m_pMenuFollowCamera     = std::make_unique<pangolin::Var<bool>>("menu.Follow Camera", true, true);
  m_pMenuShowPoints       = std::make_unique<pangolin::Var<bool>>("menu.Show Points", true, true);
  m_pMenuShowKeyFrames    = std::make_unique<pangolin::Var<bool>>("menu.Show KeyFrames", true, true);
  m_pMenuShowGraph        = std::make_unique<pangolin::Var<bool>>("menu.Show Graph", true, true);
  m_pMenuLocalizationMode = std::make_unique<pangolin::Var<bool>>("menu.Localization Mode", false, true);
  m_pMenuReset            = std::make_unique<pangolin::Var<bool>>("menu.Reset", false, false);

  // Define Camera Render Object (for view / scene browsing)
  mCameraState =  pangolin::OpenGlRenderState(pangolin::ProjectionMatrix(1024, 768, mViewpointF, mViewpointF, 512, 389, 0.1, 1000),
                                              pangolin::ModelViewLookAt(mViewpointX, mViewpointY, mViewpointZ, 0, 0, 0, 0.0, -1.0, 0.0));

  mTwc.SetIdentity();

  cv::namedWindow("ORB-SLAM2: Current Frame"); // TODO(Hussein): I don't like this
}

void PangolinViewer::preDraw() noexcept {}

void PangolinViewer::draw() noexcept {
  mpMapDrawer->GetCurrentOpenGLCameraMatrix(mTwc);

  if((*m_pMenuFollowCamera) && bFollow) {
    mCameraState.Follow(mTwc);
  } else if((*m_pMenuFollowCamera) && !bFollow) {
    mCameraState.SetModelViewMatrix(pangolin::ModelViewLookAt(mViewpointX, mViewpointY, mViewpointZ, 0, 0, 0, 0.0, -1.0, 0.0));
    mCameraState.Follow(mTwc);
    bFollow = true;
  } else if(!(*m_pMenuFollowCamera) && bFollow) {
    bFollow = false;
  }

  /*
    if(menuLocalizationMode && !bLocalizationMode) {
      mpSystem->ActivateLocalizationMode();
      bLocalizationMode = true;
    } else if(!menuLocalizationMode && bLocalizationMode) {
      mpSystem->DeactivateLocalizationMode();
      bLocalizationMode = false;
    }
    */

  // NOTE(Hussein): Not the best way
  // Add named OpenGL viewport to window and provide 3D Handler
  static auto& camera = pangolin::CreateDisplay().SetBounds(0.0, 1.0, pangolin::Attach::Pix(175), 1.0, -1024.0F / 768.0F).SetHandler(new pangolin::Handler3D(mCameraState));

  camera.Activate(mCameraState);
  glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
  mpMapDrawer->DrawCurrentCamera(mTwc);
  if((*m_pMenuShowKeyFrames) || (*m_pMenuShowGraph)) {
    mpMapDrawer->DrawKeyFrames((*m_pMenuShowKeyFrames), (*m_pMenuShowGraph));
  }

  if((*m_pMenuShowPoints)) {
    mpMapDrawer->DrawMapPoints();
  }

  pangolin::FinishFrame();
}

void PangolinViewer::afterDraw() noexcept {
  // cv::Mat im = mpFrameDrawer->DrawFrame();
  // cv::imshow("ORB-SLAM2: Current Frame", im);
  //cv::waitKey(mT);

  if((*m_pMenuReset)) {
    (*m_pMenuShowGraph)        = true;
    (*m_pMenuShowKeyFrames)    = true;
    (*m_pMenuShowPoints)       = true;
    (*m_pMenuLocalizationMode) = false;

    /*
      if(bLocalizationMode) {
        mpSystem->DeactivateLocalizationMode();
      }
    */

    bLocalizationMode = false;
    bFollow = true;
    (*m_pMenuFollowCamera) = true;
    // mpSystem->Reset();
    (*m_pMenuReset) = false;
  }

  if(stop()) {
    while(isStopped()) {
      usleep(3000);
    }
  }
}

void PangolinViewer::cleanup() noexcept {}

}  // namespace ORB_SLAM2
