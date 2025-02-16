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
#include "Converter.hpp"

namespace ORB_SLAM2 {
namespace Converter {

std::vector<cv::Mat> toDescriptorVector(const cv::Mat &Descriptors) {
  std::vector<cv::Mat> vDesc;
  vDesc.reserve(static_cast<size_t>(Descriptors.rows));
  for(int j = 0; j < Descriptors.rows; j++) {
    vDesc.push_back(Descriptors.row(j));
  }

  return vDesc;
}

g2o::SE3Quat toSE3Quat(const cv::Mat &cvT) {
  Eigen::Matrix<double, 3, 3> R;
  R << static_cast<double>(cvT.at<float>(0, 0)), static_cast<double>(cvT.at<float>(0, 1)), static_cast<double>(cvT.at<float>(0, 2)),
    static_cast<double>(cvT.at<float>(1, 0)), static_cast<double>(cvT.at<float>(1, 1)), static_cast<double>(cvT.at<float>(1, 2)),
    static_cast<double>(cvT.at<float>(2, 0)), static_cast<double>(cvT.at<float>(2, 1)), static_cast<double>(cvT.at<float>(2, 2));

  const Eigen::Matrix<double, 3, 1> t(
    static_cast<double>(cvT.at<float>(0, 3)), static_cast<double>(cvT.at<float>(1, 3)), static_cast<double>(cvT.at<float>(2, 3)));

  return g2o::SE3Quat(R, t);
}

cv::Mat toCvMat(const g2o::SE3Quat &SE3) {
  const Eigen::Matrix<double, 4, 4> eigMat = SE3.to_homogeneous_matrix();
  return toCvMat(eigMat);
}

cv::Mat toCvMat(const g2o::Sim3 &Sim3) {
  const Eigen::Matrix3d eigR = Sim3.rotation().toRotationMatrix();
  const Eigen::Vector3d &eigt = Sim3.translation();
  const double s = Sim3.scale();
  return toCvSE3(s * eigR, eigt);
}

cv::Mat toCvMat(const Eigen::Matrix<double, 4, 4> &m) {
  cv::Mat cvMat(4, 4, CV_32F);
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      cvMat.at<float>(i, j) = static_cast<float>(m(i, j));
    }
  }

  return cvMat.clone();
}

cv::Mat toCvMat(const Eigen::Matrix3d &m) {
  cv::Mat cvMat(3, 3, CV_32F);
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      cvMat.at<float>(i, j) = static_cast<float>(m(i, j));
    }
  }

  return cvMat.clone();
}

cv::Mat toCvMat(const Eigen::Matrix<double, 3, 1> &m) {
  cv::Mat cvMat(3, 1, CV_32F);
  for(int i = 0; i < 3; i++) {
    cvMat.at<float>(i) = static_cast<float>(m(i));
  }

  return cvMat.clone();
}

cv::Mat toCvSE3(const Eigen::Matrix<double, 3, 3> &R, const Eigen::Matrix<double, 3, 1> &t) {
  cv::Mat cvMat = cv::Mat::eye(4, 4, CV_32F);
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      cvMat.at<float>(i, j) = static_cast<float>(R(i, j));
    }
  }

  for(int i = 0; i < 3; i++) {
    cvMat.at<float>(i, 3) = static_cast<float>(t(i));
  }

  return cvMat.clone();
}

Eigen::Matrix<double, 3, 1> toVector3d(const cv::Mat &cvVector) {
  Eigen::Matrix<double, 3, 1> v;
  v << static_cast<double>(cvVector.at<float>(0)), static_cast<double>(cvVector.at<float>(1)), static_cast<double>(cvVector.at<float>(2));

  return v;
}

Eigen::Matrix<double, 3, 1> toVector3d(const cv::Point3f &cvPoint) {
  Eigen::Matrix<double, 3, 1> v;
  v << static_cast<double>(cvPoint.x), static_cast<double>(cvPoint.y), static_cast<double>(cvPoint.z);

  return v;
}

Eigen::Matrix<double, 3, 3> toMatrix3d(const cv::Mat &cvMat3) {
  Eigen::Matrix<double, 3, 3> M;

  M << static_cast<double>(cvMat3.at<float>(0, 0)), static_cast<double>(cvMat3.at<float>(0, 1)),
    static_cast<double>(cvMat3.at<float>(0, 2)), static_cast<double>(cvMat3.at<float>(1, 0)),
    static_cast<double>(cvMat3.at<float>(1, 1)), static_cast<double>(cvMat3.at<float>(1, 2)),
    static_cast<double>(cvMat3.at<float>(2, 0)), static_cast<double>(cvMat3.at<float>(2, 1)),
    static_cast<double>(cvMat3.at<float>(2, 2));

  return M;
}

std::vector<float> toQuaternion(const cv::Mat &M) {
  const Eigen::Matrix<double, 3, 3> eigMat = toMatrix3d(M);
  const Eigen::Quaterniond q(eigMat);

  return {static_cast<float>(q.x()), static_cast<float>(q.y()), static_cast<float>(q.z()), static_cast<float>(q.w())};
}

}  // namespace Converter
}  // namespace ORB_SLAM2
