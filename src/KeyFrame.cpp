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
#include "KeyFrame.hpp"
//
#include "Map.hpp"
#include "Frame.hpp"
#include "MapPoint.hpp"
#include "Converter.hpp"
#include "ORBmatcher.hpp"
#include "ORBextractor.hpp"
#include "KeyFrameDatabase.hpp"

namespace ORB_SLAM2 {

long unsigned int KeyFrame::nNextId = 0;

KeyFrame::KeyFrame(Frame &F, Map *pMap, KeyFrameDatabase *pKFDB) :
    mnFrameId(F.mnId), mTimeStamp(F.mTimeStamp), mnGridCols(FRAME_GRID_COLS), mnGridRows(FRAME_GRID_ROWS),
    mfGridElementWidthInv(F.mfGridElementWidthInv), mfGridElementHeightInv(F.mfGridElementHeightInv), mnTrackReferenceForFrame(0),
    mnFuseTargetForKF(0), mnBALocalForKF(0), mnBAFixedForKF(0), mnLoopQuery(0), mnLoopWords(0), mnRelocQuery(0), mnRelocWords(0),
    mnBAGlobalForKF(0), fx(F.fx), fy(F.fy), cx(F.cx), cy(F.cy), invfx(F.invfx), invfy(F.invfy), mbf(F.mbf), mb(F.mb),
    mThDepth(F.mThDepth), N(F.N), mvKeys(F.mvKeys), mvKeysUn(F.mvKeysUn), mvuRight(F.mvuRight), mvDepth(F.mvDepth),
    mDescriptors(F.mDescriptors.clone()), mBowVec(F.mBowVec), mFeatVec(F.mFeatVec), mnScaleLevels(F.mnScaleLevels),
    mfScaleFactor(F.mfScaleFactor), mfLogScaleFactor(F.mfLogScaleFactor), mvScaleFactors(F.mvScaleFactors),
    mvLevelSigma2(F.mvLevelSigma2), mvInvLevelSigma2(F.mvInvLevelSigma2), mnMinX(F.mnMinX), mnMinY(F.mnMinY), mnMaxX(F.mnMaxX),
    mnMaxY(F.mnMaxY), mK(F.mK), mvpMapPoints(F.mvpMapPoints), mpKeyFrameDB(pKFDB), mpORBvocabulary(F.mpORBvocabulary),
    mbFirstConnection(true), mpParent(nullptr), mbNotErase(false), mbToBeErased(false), mbBad(false), mHalfBaseline(F.mb / 2), mpMap(pMap) {
  mnId = nNextId++;

  mGrid.resize(mnGridCols);
  for(int i = 0; i < mnGridCols; i++) {
    mGrid[i].resize(mnGridRows);
    for(int j = 0; j < mnGridRows; j++)
      mGrid[i][j] = F.mGrid[i][j];
  }

  SetPose(F.mTcw);
}

void KeyFrame::ComputeBoW() {
  if(mBowVec.empty() || mFeatVec.empty()) {
    vector<cv::Mat> vCurrentDesc = Converter::toDescriptorVector(mDescriptors);
    // Feature vector associate features with nodes in the 4th level (from leaves up)
    // We assume the vocabulary tree has 6 levels, change the 4 otherwise
    mpORBvocabulary->transform(vCurrentDesc, mBowVec, mFeatVec, 4);
  }
}

void KeyFrame::SetPose(const cv::Mat &Tcw_) {
  unique_lock<mutex> lock(mMutexPose);
  Tcw_.copyTo(Tcw);
  cv::Mat Rcw = Tcw.rowRange(0, 3).colRange(0, 3);
  cv::Mat tcw = Tcw.rowRange(0, 3).col(3);
  cv::Mat Rwc = Rcw.t();
  Ow = -Rwc * tcw;

  Twc = cv::Mat::eye(4, 4, Tcw.type());
  Rwc.copyTo(Twc.rowRange(0, 3).colRange(0, 3));
  Ow.copyTo(Twc.rowRange(0, 3).col(3));
  cv::Mat center = (cv::Mat_<float>(4, 1) << mHalfBaseline, 0, 0, 1);
  Cw = Twc * center;
}

cv::Mat KeyFrame::GetPose() {
  unique_lock<mutex> lock(mMutexPose);
  return Tcw.clone();
}

cv::Mat KeyFrame::GetPoseInverse() {
  unique_lock<mutex> lock(mMutexPose);
  return Twc.clone();
}

cv::Mat KeyFrame::GetCameraCenter() {
  unique_lock<mutex> lock(mMutexPose);
  return Ow.clone();
}

cv::Mat KeyFrame::GetStereoCenter() {
  unique_lock<mutex> lock(mMutexPose);
  return Cw.clone();
}

cv::Mat KeyFrame::GetRotation() {
  unique_lock<mutex> lock(mMutexPose);
  return Tcw.rowRange(0, 3).colRange(0, 3).clone();
}

cv::Mat KeyFrame::GetTranslation() {
  unique_lock<mutex> lock(mMutexPose);
  return Tcw.rowRange(0, 3).col(3).clone();
}

void KeyFrame::AddConnection(KeyFrame *pKF, const int &weight) {
  {
    std::unique_lock<std::mutex> lock(mMutexConnections);
    if(!mConnectedKeyFrameWeights.count(pKF))
      mConnectedKeyFrameWeights[pKF] = weight;
    else if(mConnectedKeyFrameWeights[pKF] != weight)
      mConnectedKeyFrameWeights[pKF] = weight;
    else
      return;
  }

  UpdateBestCovisibles();
}

void KeyFrame::UpdateBestCovisibles() {
  unique_lock<mutex> lock(mMutexConnections);
  vector<pair<int, KeyFrame *> > vPairs;
  vPairs.reserve(mConnectedKeyFrameWeights.size());
  for(auto & mConnectedKeyFrameWeight : mConnectedKeyFrameWeights)
    vPairs.emplace_back(mConnectedKeyFrameWeight.second, mConnectedKeyFrameWeight.first);

  sort(vPairs.begin(), vPairs.end());
  list<KeyFrame *> lKFs;
  list<int> lWs;
  for(auto & vPair : vPairs) {
    lKFs.push_front(vPair.second);
    lWs.push_front(vPair.first);
  }

  mvpOrderedConnectedKeyFrames = vector<KeyFrame *>(lKFs.begin(), lKFs.end());
  mvOrderedWeights = vector<int>(lWs.begin(), lWs.end());
}

set<KeyFrame *> KeyFrame::GetConnectedKeyFrames() {
  std::unique_lock<std::mutex> lock(mMutexConnections);
  set<KeyFrame *> s;
  for(auto & mConnectedKeyFrameWeight : mConnectedKeyFrameWeights) {
    s.insert(mConnectedKeyFrameWeight.first);
  }
  return s;
}

vector<KeyFrame *> KeyFrame::GetVectorCovisibleKeyFrames() {
  unique_lock<mutex> lock(mMutexConnections);
  return mvpOrderedConnectedKeyFrames;
}

vector<KeyFrame *> KeyFrame::GetBestCovisibilityKeyFrames(int count) {
  std::unique_lock<std::mutex> lock(mMutexConnections);
  if(static_cast<int>(mvpOrderedConnectedKeyFrames.size()) < count) {
    return mvpOrderedConnectedKeyFrames;
  }
  return vector<KeyFrame *>(mvpOrderedConnectedKeyFrames.begin(), mvpOrderedConnectedKeyFrames.begin() + count);
}

vector<KeyFrame *> KeyFrame::GetCovisiblesByWeight(const int &w) {
  unique_lock<mutex> lock(mMutexConnections);

  if(mvpOrderedConnectedKeyFrames.empty())
    return vector<KeyFrame *>();

  auto it = upper_bound(mvOrderedWeights.begin(), mvOrderedWeights.end(), w, KeyFrame::weightComp);
  if(it == mvOrderedWeights.end())
    return vector<KeyFrame *>();
  else {
    int n = it - mvOrderedWeights.begin();
    return vector<KeyFrame *>(mvpOrderedConnectedKeyFrames.begin(), mvpOrderedConnectedKeyFrames.begin() + n);
  }
}

int KeyFrame::GetWeight(KeyFrame *pKF) {
  unique_lock<mutex> lock(mMutexConnections);
  if(mConnectedKeyFrameWeights.count(pKF))
    return mConnectedKeyFrameWeights[pKF];
  else
    return 0;
}

void KeyFrame::AddMapPoint(MapPoint *pMP, const size_t &idx) {
  unique_lock<mutex> lock(mMutexFeatures);
  mvpMapPoints[idx] = pMP;
}

void KeyFrame::EraseMapPointMatch(const size_t &idx) {
  std::unique_lock<std::mutex> lock(mMutexFeatures);
  mvpMapPoints[idx] = nullptr;
}

void KeyFrame::EraseMapPointMatch(MapPoint *pMP) {
  int idx = pMP->GetIndexInKeyFrame(this);
  if(idx >= 0) {
    mvpMapPoints[idx] = nullptr;
  }
}

void KeyFrame::ReplaceMapPointMatch(const size_t &idx, MapPoint *pMP) { mvpMapPoints[idx] = pMP; }

set<MapPoint *> KeyFrame::GetMapPoints() {
  unique_lock<mutex> lock(mMutexFeatures);
  set<MapPoint *> s;
  for(auto & mvpMapPoint : mvpMapPoints) {
    if(!mvpMapPoint)
      continue;
    MapPoint *pMP = mvpMapPoint;
    if(!pMP->isBad())
      s.insert(pMP);
  }
  return s;
}

int KeyFrame::TrackedMapPoints(const int &minObs) {
  unique_lock<mutex> lock(mMutexFeatures);

  int nPoints = 0;
  const bool bCheckObs = minObs > 0;
  for(int i = 0; i < N; i++) {
    MapPoint *pMP = mvpMapPoints[i];
    if(pMP) {
      if(!pMP->isBad()) {
        if(bCheckObs) {
          if(mvpMapPoints[i]->Observations() >= minObs)
            nPoints++;
        } else
          nPoints++;
      }
    }
  }

  return nPoints;
}

vector<MapPoint *> KeyFrame::GetMapPointMatches() {
  unique_lock<mutex> lock(mMutexFeatures);
  return mvpMapPoints;
}

MapPoint *KeyFrame::GetMapPoint(const size_t &idx) {
  unique_lock<mutex> lock(mMutexFeatures);
  return mvpMapPoints[idx];
}

void KeyFrame::UpdateConnections() {
  map<KeyFrame *, int> KFcounter;

  vector<MapPoint *> vpMP;

  {
    unique_lock<mutex> lockMPs(mMutexFeatures);
    vpMP = mvpMapPoints;
  }

  //For all map points in keyframe check in which other keyframes are they seen
  //Increase counter for those keyframes
  for(auto pMP : vpMP) {
    if(!pMP)
      continue;

    if(pMP->isBad())
      continue;

    map<KeyFrame *, size_t> observations = pMP->GetObservations();

    for(auto & observation : observations) {
      if(observation.first->mnId == mnId)
        continue;
      KFcounter[observation.first]++;
    }
  }

  // This should not happen
  if(KFcounter.empty())
    return;

  //If the counter is greater than threshold add connection
  //In case no keyframe counter is over threshold add the one with maximum counter
  int nmax = 0;
  KeyFrame *pKFmax = nullptr;
  int th = 15;

  vector<pair<int, KeyFrame *> > vPairs;
  vPairs.reserve(KFcounter.size());
  for(auto & mit : KFcounter) {
    if(mit.second > nmax) {
      nmax = mit.second;
      pKFmax = mit.first;
    }
    if(mit.second >= th) {
      vPairs.emplace_back(mit.second, mit.first);
      (mit.first)->AddConnection(this, mit.second);
    }
  }

  if(vPairs.empty()) {
    vPairs.emplace_back(nmax, pKFmax);
    pKFmax->AddConnection(this, nmax);
  }

  sort(vPairs.begin(), vPairs.end());
  list<KeyFrame *> lKFs;
  list<int> lWs;
  for(auto & vPair : vPairs) {
    lKFs.push_front(vPair.second);
    lWs.push_front(vPair.first);
  }

  {
    unique_lock<mutex> lockCon(mMutexConnections);

    // mspConnectedKeyFrames = spConnectedKeyFrames;
    mConnectedKeyFrameWeights = KFcounter;
    mvpOrderedConnectedKeyFrames = vector<KeyFrame *>(lKFs.begin(), lKFs.end());
    mvOrderedWeights = vector<int>(lWs.begin(), lWs.end());

    if(mbFirstConnection && mnId != 0) {
      mpParent = mvpOrderedConnectedKeyFrames.front();
      mpParent->AddChild(this);
      mbFirstConnection = false;
    }
  }
}

void KeyFrame::AddChild(KeyFrame *pKF) {
  unique_lock<mutex> lockCon(mMutexConnections);
  mspChildrens.insert(pKF);
}

void KeyFrame::EraseChild(KeyFrame *pKF) {
  unique_lock<mutex> lockCon(mMutexConnections);
  mspChildrens.erase(pKF);
}

void KeyFrame::ChangeParent(KeyFrame *pKF) {
  unique_lock<mutex> lockCon(mMutexConnections);
  mpParent = pKF;
  pKF->AddChild(this);
}

set<KeyFrame *> KeyFrame::GetChilds() {
  unique_lock<mutex> lockCon(mMutexConnections);
  return mspChildrens;
}

KeyFrame *KeyFrame::GetParent() {
  unique_lock<mutex> lockCon(mMutexConnections);
  return mpParent;
}

bool KeyFrame::hasChild(KeyFrame *pKF) {
  unique_lock<mutex> lockCon(mMutexConnections);
  return mspChildrens.count(pKF);
}

void KeyFrame::AddLoopEdge(KeyFrame *pKF) {
  unique_lock<mutex> lockCon(mMutexConnections);
  mbNotErase = true;
  mspLoopEdges.insert(pKF);
}

set<KeyFrame *> KeyFrame::GetLoopEdges() {
  unique_lock<mutex> lockCon(mMutexConnections);
  return mspLoopEdges;
}

void KeyFrame::SetNotErase() {
  unique_lock<mutex> lock(mMutexConnections);
  mbNotErase = true;
}

void KeyFrame::SetErase() {
  {
    unique_lock<mutex> lock(mMutexConnections);
    if(mspLoopEdges.empty()) {
      mbNotErase = false;
    }
  }

  if(mbToBeErased) {
    SetBadFlag();
  }
}

void KeyFrame::SetBadFlag() {

  {
    unique_lock<mutex> lock(mMutexConnections);
    if(mnId == 0)
      return;
    else if(mbNotErase) {
      mbToBeErased = true;
      return;
    }
  }

  for(auto & mConnectedKeyFrameWeight : mConnectedKeyFrameWeights)
    mConnectedKeyFrameWeight.first->EraseConnection(this);

  for(auto & mvpMapPoint : mvpMapPoints)
    if(mvpMapPoint)
      mvpMapPoint->EraseObservation(this);
  {
    unique_lock<mutex> lock(mMutexConnections);
    unique_lock<mutex> lock1(mMutexFeatures);

    mConnectedKeyFrameWeights.clear();
    mvpOrderedConnectedKeyFrames.clear();

    // Update Spanning Tree
    set<KeyFrame *> sParentCandidates;
    sParentCandidates.insert(mpParent);

    // Assign at each iteration one children with a parent (the pair with highest covisibility weight)
    // Include that children as new parent candidate for the rest
    while(!mspChildrens.empty()) {
      bool bContinue = false;

      int max = -1;
      KeyFrame *pC;
      KeyFrame *pP;

      for(auto pKF : mspChildrens) {
        if(pKF->isBad())
          continue;

        // Check if a parent candidate is connected to the keyframe
        vector<KeyFrame *> vpConnected = pKF->GetVectorCovisibleKeyFrames();
        for(auto & i : vpConnected) {
          for(auto sParentCandidate : sParentCandidates) {
            if(i->mnId == sParentCandidate->mnId) {
              int w = pKF->GetWeight(i);
              if(w > max) {
                pC = pKF;
                pP = i;
                max = w;
                bContinue = true;
              }
            }
          }
        }
      }

      if(bContinue) {
        pC->ChangeParent(pP);
        sParentCandidates.insert(pC);
        mspChildrens.erase(pC);
      } else
        break;
    }

    // If a children has no covisibility links with any parent candidate, assign to the original parent of this KF
    if(!mspChildrens.empty())
      for(auto mspChildren : mspChildrens) {
        mspChildren->ChangeParent(mpParent);
      }

    mpParent->EraseChild(this);
    mTcp = Tcw * mpParent->GetPoseInverse();
    mbBad = true;
  }

  mpMap->EraseKeyFrame(this);
  mpKeyFrameDB->erase(this);
}

bool KeyFrame::isBad() {
  unique_lock<mutex> lock(mMutexConnections);
  return mbBad;
}

void KeyFrame::EraseConnection(KeyFrame *pKF) {
  bool bUpdate = false;
  {
    unique_lock<mutex> lock(mMutexConnections);
    if(mConnectedKeyFrameWeights.count(pKF)) {
      mConnectedKeyFrameWeights.erase(pKF);
      bUpdate = true;
    }
  }

  if(bUpdate)
    UpdateBestCovisibles();
}

vector<size_t> KeyFrame::GetFeaturesInArea(const float &x, const float &y, const float &r) const {
  vector<size_t> vIndices;
  vIndices.reserve(N);

  const int nMinCellX = max(0, static_cast<int>(floor((x - mnMinX - r)) * mfGridElementWidthInv));
  if(nMinCellX >= mnGridCols)
    return vIndices;

  const int nMaxCellX = min(static_cast<int>(mnGridCols) - 1, static_cast<int>(ceil((x - mnMinX + r) * mfGridElementWidthInv)));
  if(nMaxCellX < 0)
    return vIndices;

  const int nMinCellY = max(0, static_cast<int>(floor((y - mnMinY - r) * mfGridElementHeightInv)));
  if(nMinCellY >= mnGridRows)
    return vIndices;

  const int nMaxCellY = min(static_cast<int>(mnGridRows) - 1, static_cast<int>(ceil((y - mnMinY + r) * mfGridElementHeightInv)));
  if(nMaxCellY < 0)
    return vIndices;

  for(int ix = nMinCellX; ix <= nMaxCellX; ix++) {
    for(int iy = nMinCellY; iy <= nMaxCellY; iy++) {
      const vector<size_t> vCell = mGrid[ix][iy];
      for(unsigned long j : vCell) {
        const cv::KeyPoint &kpUn = mvKeysUn[j];
        const float distx = kpUn.pt.x - x;
        const float disty = kpUn.pt.y - y;

        if(fabs(distx) < r && fabs(disty) < r)
          vIndices.push_back(j);
      }
    }
  }

  return vIndices;
}

bool KeyFrame::IsInImage(const float &x, const float &y) const {
  return (x >= mnMinX && x < mnMaxX && y >= mnMinY && y < mnMaxY);
}

cv::Mat KeyFrame::UnprojectStereo(int i) {
  const float z = mvDepth[i];
  if(z > 0) {
    const float u = mvKeys[i].pt.x;
    const float v = mvKeys[i].pt.y;
    const float x = (u - cx) * z * invfx;
    const float y = (v - cy) * z * invfy;
    cv::Mat x3Dc = (cv::Mat_<float>(3, 1) << x, y, z);

    unique_lock<mutex> lock(mMutexPose);
    return Twc.rowRange(0, 3).colRange(0, 3) * x3Dc + Twc.rowRange(0, 3).col(3);
  } else
    return cv::Mat();
}

float KeyFrame::ComputeSceneMedianDepth(const int q) {
  vector<MapPoint *> vpMapPoints;
  cv::Mat Tcw_;
  {
    unique_lock<mutex> lock(mMutexFeatures);
    unique_lock<mutex> lock2(mMutexPose);
    //vpMapPoints = mvpMapPoints;
    Tcw_ = Tcw.clone();
  }

  vector<float> vDepths;
  vDepths.reserve(N);
  cv::Mat Rcw2 = Tcw_.row(2).colRange(0, 3);
  Rcw2 = Rcw2.t();
  float zcw = Tcw_.at<float>(2, 3);
  for(int i = 0; i < N; i++) {
    if(mvpMapPoints[i]) {
      MapPoint *pMP = mvpMapPoints[i];
      cv::Mat x3Dw = pMP->GetWorldPos();
      float z = Rcw2.dot(x3Dw) + zcw;
      vDepths.push_back(z);
    }
  }

  sort(vDepths.begin(), vDepths.end());

  return vDepths[(vDepths.size() - 1) / q];
}

}  // namespace ORB_SLAM2
