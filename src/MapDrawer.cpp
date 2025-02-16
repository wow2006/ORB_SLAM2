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
#include "MapDrawer.hpp"
//
#include "Map.hpp"
#include "MapPoint.hpp"
#include "KeyFrame.hpp"

namespace ORB_SLAM2 {

MapDrawer::MapDrawer(Map *pMap, const string &strSettingPath) : mpMap(pMap) {
  cv::FileStorage fSettings(strSettingPath, cv::FileStorage::READ);

  mKeyFrameSize = fSettings["Viewer.KeyFrameSize"];
  mKeyFrameLineWidth = fSettings["Viewer.KeyFrameLineWidth"];
  mGraphLineWidth = fSettings["Viewer.GraphLineWidth"];
  mPointSize = fSettings["Viewer.PointSize"];
  mCameraSize = fSettings["Viewer.CameraSize"];
  mCameraLineWidth = fSettings["Viewer.CameraLineWidth"];
}

void MapDrawer::DrawMapPoints() const {
  const vector<MapPoint *> &vpMPs = mpMap->GetAllMapPoints();
  const vector<MapPoint *> &vpRefMPs = mpMap->GetReferenceMapPoints();

  set<MapPoint *> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());

  if(vpMPs.empty())
    return;

  glPointSize(mPointSize);
  glBegin(GL_POINTS);
  glColor3f(0.0, 0.0, 0.0);

  for(auto vpMP : vpMPs) {
    if(vpMP->isBad() || spRefMPs.count(vpMP))
      continue;
    cv::Mat pos = vpMP->GetWorldPos();
    glVertex3f(pos.at<float>(0), pos.at<float>(1), pos.at<float>(2));
  }
  glEnd();

  glPointSize(mPointSize);
  glBegin(GL_POINTS);
  glColor3f(1.0, 0.0, 0.0);

  for(auto spRefMP : spRefMPs) {
    if(spRefMP->isBad())
      continue;
    cv::Mat pos = spRefMP->GetWorldPos();
    glVertex3f(pos.at<float>(0), pos.at<float>(1), pos.at<float>(2));
  }

  glEnd();
}

void MapDrawer::DrawKeyFrames(bool bDrawKF, bool bDrawGraph) const {
  const float &w = mKeyFrameSize;
  const float h = w * 0.75;
  const float z = w * 0.6;

  const vector<KeyFrame *> vpKFs = mpMap->GetAllKeyFrames();

  if(bDrawKF) {
    for(auto pKF : vpKFs) {
      cv::Mat Twc = pKF->GetPoseInverse().t();

      glPushMatrix();

      glMultMatrixf(Twc.ptr<GLfloat>(0));

      glLineWidth(mKeyFrameLineWidth);
      glColor3f(0.0f, 0.0f, 1.0f);
      glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(w, h, z);
      glVertex3f(0, 0, 0);
      glVertex3f(w, -h, z);
      glVertex3f(0, 0, 0);
      glVertex3f(-w, -h, z);
      glVertex3f(0, 0, 0);
      glVertex3f(-w, h, z);

      glVertex3f(w, h, z);
      glVertex3f(w, -h, z);

      glVertex3f(-w, h, z);
      glVertex3f(-w, -h, z);

      glVertex3f(-w, h, z);
      glVertex3f(w, h, z);

      glVertex3f(-w, -h, z);
      glVertex3f(w, -h, z);
      glEnd();

      glPopMatrix();
    }
  }

  if(bDrawGraph) {
    glLineWidth(mGraphLineWidth);
    glColor4f(0.0f, 1.0f, 0.0f, 0.6f);
    glBegin(GL_LINES);

    for(auto vpKF : vpKFs) {
      // Covisibility Graph
      const vector<KeyFrame *> vCovKFs = vpKF->GetCovisiblesByWeight(100);
      cv::Mat Ow = vpKF->GetCameraCenter();
      if(!vCovKFs.empty()) {
        for(auto vCovKF : vCovKFs) {
          if(vCovKF->mnId < vpKF->mnId)
            continue;
          cv::Mat Ow2 = vCovKF->GetCameraCenter();
          glVertex3f(Ow.at<float>(0), Ow.at<float>(1), Ow.at<float>(2));
          glVertex3f(Ow2.at<float>(0), Ow2.at<float>(1), Ow2.at<float>(2));
        }
      }

      // Spanning tree
      KeyFrame *pParent = vpKF->GetParent();
      if(pParent) {
        cv::Mat Owp = pParent->GetCameraCenter();
        glVertex3f(Ow.at<float>(0), Ow.at<float>(1), Ow.at<float>(2));
        glVertex3f(Owp.at<float>(0), Owp.at<float>(1), Owp.at<float>(2));
      }

      // Loops
      set<KeyFrame *> sLoopKFs = vpKF->GetLoopEdges();
      for(auto sLoopKF : sLoopKFs) {
        if(sLoopKF->mnId < vpKF->mnId)
          continue;
        cv::Mat Owl = sLoopKF->GetCameraCenter();
        glVertex3f(Ow.at<float>(0), Ow.at<float>(1), Ow.at<float>(2));
        glVertex3f(Owl.at<float>(0), Owl.at<float>(1), Owl.at<float>(2));
      }
    }

    glEnd();
  }
}

void MapDrawer::DrawCurrentCamera(pangolin::OpenGlMatrix &Twc) const {
  const float &w = mCameraSize;
  const float h = w * 0.75;
  const float z = w * 0.6;

  glPushMatrix();

#ifdef HAVE_GLES
  glMultMatrixf(Twc.m);
#else
  glMultMatrixd(Twc.m);
#endif

  glLineWidth(mCameraLineWidth);
  glColor3f(0.0f, 1.0f, 0.0f);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(w, h, z);
  glVertex3f(0, 0, 0);
  glVertex3f(w, -h, z);
  glVertex3f(0, 0, 0);
  glVertex3f(-w, -h, z);
  glVertex3f(0, 0, 0);
  glVertex3f(-w, h, z);

  glVertex3f(w, h, z);
  glVertex3f(w, -h, z);

  glVertex3f(-w, h, z);
  glVertex3f(-w, -h, z);

  glVertex3f(-w, h, z);
  glVertex3f(w, h, z);

  glVertex3f(-w, -h, z);
  glVertex3f(w, -h, z);
  glEnd();

  glPopMatrix();
}

void MapDrawer::SetCurrentCameraPose(const cv::Mat &Tcw) {
  unique_lock<mutex> lock(mMutexCamera);
  mCameraPose = Tcw.clone();
}

void MapDrawer::GetCurrentOpenGLCameraMatrix(pangolin::OpenGlMatrix &M) {
  if(!mCameraPose.empty()) {
    cv::Mat Rwc(3, 3, CV_32F);
    cv::Mat twc(3, 1, CV_32F);
    {
      unique_lock<mutex> lock(mMutexCamera);
      Rwc = mCameraPose.rowRange(0, 3).colRange(0, 3).t();
      twc = -Rwc * mCameraPose.rowRange(0, 3).col(3);
    }

    M.m[0] = Rwc.at<float>(0, 0);
    M.m[1] = Rwc.at<float>(1, 0);
    M.m[2] = Rwc.at<float>(2, 0);
    M.m[3] = 0.0;

    M.m[4] = Rwc.at<float>(0, 1);
    M.m[5] = Rwc.at<float>(1, 1);
    M.m[6] = Rwc.at<float>(2, 1);
    M.m[7] = 0.0;

    M.m[8] = Rwc.at<float>(0, 2);
    M.m[9] = Rwc.at<float>(1, 2);
    M.m[10] = Rwc.at<float>(2, 2);
    M.m[11] = 0.0;

    M.m[12] = twc.at<float>(0);
    M.m[13] = twc.at<float>(1);
    M.m[14] = twc.at<float>(2);
    M.m[15] = 1.0;
  } else
    M.SetIdentity();
}

}  // namespace ORB_SLAM2
