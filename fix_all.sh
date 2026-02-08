#!/bin/bash
echo "=== 快速修复Eigen问题 ==="

echo "1. 创建kalman_filter.hpp..."
cat > include/armor_detector/kalman_filter.hpp << 'HPP'
#pragma once

#include <opencv2/opencv.hpp>

namespace rm_auto_aim
{
class KalmanFilter
{
public:
  KalmanFilter();
  void init(const cv::Point2f& initial_pos);
  cv::Point2f predict();
  void update(const cv::Point2f& measurement);
  
  bool isInitialized() const { return initialized_; }
  cv::Point2f getPrediction() const { return prediction_; }

private:
  bool initialized_ = false;
  cv::Point2f prediction_;
  
  cv::KalmanFilter kf_;
  int stateSize_ = 4;
  int measSize_ = 2;
  int contrSize_ = 0;
  cv::Mat measurement_;
  
  void initKalmanFilter();
};
}
HPP

echo "2. 创建kalman_filter.cpp..."
cat > src/kalman_filter.cpp << 'CPP'
#include <cmath>
#include "armor_detector/kalman_filter.hpp"

namespace rm_auto_aim
{

KalmanFilter::KalmanFilter()
  : kf_(stateSize_, measSize_, contrSize_, CV_32F),
    initialized_(false)
{
  initKalmanFilter();
}

void KalmanFilter::initKalmanFilter()
{
  cv::setIdentity(kf_.transitionMatrix);
  kf_.transitionMatrix.at<float>(0, 1) = 0.033f;
  kf_.transitionMatrix.at<float>(2, 3) = 0.033f;
  
  kf_.measurementMatrix = cv::Mat::zeros(measSize_, stateSize_, CV_32F);
  kf_.measurementMatrix.at<float>(0, 0) = 1.0f;
  kf_.measurementMatrix.at<float>(1, 2) = 1.0f;
  
  kf_.processNoiseCov = cv::Mat::eye(stateSize_, stateSize_, CV_32F) * 0.001f;
  kf_.processNoiseCov.at<float>(0, 0) = 0.1f;
  kf_.processNoiseCov.at<float>(1, 1) = 0.5f;
  kf_.processNoiseCov.at<float>(2, 2) = 0.1f;
  kf_.processNoiseCov.at<float>(3, 3) = 0.5f;
  
  kf_.measurementNoiseCov = cv::Mat::eye(measSize_, measSize_, CV_32F) * 5.0f;
  
  cv::setIdentity(kf_.errorCovPost, cv::Scalar::all(0.1));
  
  measurement_ = cv::Mat::zeros(measSize_, 1, CV_32F);
}

void KalmanFilter::init(const cv::Point2f& initial_pos)
{
  kf_.statePost.at<float>(0) = initial_pos.x;
  kf_.statePost.at<float>(1) = 0.0f;
  kf_.statePost.at<float>(2) = initial_pos.y;
  kf_.statePost.at<float>(3) = 0.0f;
  
  initialized_ = true;
  prediction_ = initial_pos;
}

cv::Point2f KalmanFilter::predict()
{
  if (!initialized_) return cv::Point2f(0, 0);
  
  cv::Mat prediction = kf_.predict();
  prediction_.x = prediction.at<float>(0);
  prediction_.y = prediction.at<float>(2);
  
  return prediction_;
}

void KalmanFilter::update(const cv::Point2f& measurement)
{
  if (!initialized_) return;
  
  measurement_.at<float>(0) = measurement.x;
  measurement_.at<float>(1) = measurement.y;
  
  kf_.correct(measurement_);
  
  prediction_.x = kf_.statePost.at<float>(0);
  prediction_.y = kf_.statePost.at<float>(2);
}

}
CPP

echo "3. 检查CMakeLists.txt..."
if grep -q "Eigen" CMakeLists.txt; then
  echo "发现Eigen依赖，请注释掉以下行："
  echo "  find_package(Eigen3 REQUIRED)"
  echo "  include_directories(\${EIGEN3_INCLUDE_DIRS})"
else
  echo "CMakeLists.txt中没有Eigen依赖，很好！"
fi

echo "4. 创建测试图片..."
python3 -c "
import cv2, numpy as np
img = np.zeros((480,640,3), np.uint8)
cv2.rectangle(img, (280,190), (300,290), (0,0,255), -1)
cv2.rectangle(img, (340,190), (360,290), (0,0,255), -1)
cv2.imwrite('test_simple.jpg', img)
print('✅ 创建 test_simple.jpg')
"

echo "=== 修复完成！ ==="
echo ""
echo "接下来执行："
echo "1. cd build"
echo "2. make -j1"
echo "3. cd .."
echo "4. ./bin/rm_vision_newtest test_simple.jpg"
