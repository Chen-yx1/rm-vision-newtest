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
  // 状态转移矩阵 A (x, vx, y, vy)
  cv::setIdentity(kf_.transitionMatrix);
  kf_.transitionMatrix.at<float>(0, 1) = 0.033f;  // dt = 0.033s (30fps)
  kf_.transitionMatrix.at<float>(2, 3) = 0.033f;
  
  // 测量矩阵 H (只测量位置)
  kf_.measurementMatrix = cv::Mat::zeros(measSize_, stateSize_, CV_32F);
  kf_.measurementMatrix.at<float>(0, 0) = 1.0f;
  kf_.measurementMatrix.at<float>(1, 2) = 1.0f;
  
  // 过程噪声协方差矩阵 Q
  kf_.processNoiseCov = cv::Mat::eye(stateSize_, stateSize_, CV_32F) * 0.001f;
  kf_.processNoiseCov.at<float>(0, 0) = 0.1f;
  kf_.processNoiseCov.at<float>(1, 1) = 0.5f;
  kf_.processNoiseCov.at<float>(2, 2) = 0.1f;
  kf_.processNoiseCov.at<float>(3, 3) = 0.5f;
  
  // 测量噪声协方差矩阵 R
  kf_.measurementNoiseCov = cv::Mat::eye(measSize_, measSize_, CV_32F) * 5.0f;
  
  // 后验误差协方差矩阵 P
  cv::setIdentity(kf_.errorCovPost, cv::Scalar::all(0.1));
  
  // 初始化测量向量
  measurement_ = cv::Mat::zeros(measSize_, 1, CV_32F);
}

void KalmanFilter::init(const cv::Point2f& initial_pos)
{
  kf_.statePost.at<float>(0) = initial_pos.x;  // x
  kf_.statePost.at<float>(1) = 0.0f;          // vx
  kf_.statePost.at<float>(2) = initial_pos.y;  // y
  kf_.statePost.at<float>(3) = 0.0f;          // vy
  
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

} // namespace rm_auto_aim
