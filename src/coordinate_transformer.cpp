#include "armor_detector/coordinate_transformer.hpp"
#include "armor_detector/camera_calibrator.hpp"
#include <iostream>

namespace rm_auto_aim
{

CoordinateTransformer::CoordinateTransformer()
{
  camera_matrix_ = (cv::Mat_<double>(3, 3) << 
    1000.0, 0.0, 320.0,
    0.0, 1000.0, 240.0,
    0.0, 0.0, 1.0);
  dist_coeffs_ = cv::Mat::zeros(5, 1, CV_64F);
  params_initialized_ = true;
}

void CoordinateTransformer::setCameraMatrix(const cv::Mat& matrix)
{
  if (!matrix.empty()) {
    matrix.copyTo(camera_matrix_);
    params_initialized_ = true;
  }
}

void CoordinateTransformer::setDistCoeffs(const cv::Mat& coeffs)
{
  if (!coeffs.empty()) coeffs.copyTo(dist_coeffs_);
}

void CoordinateTransformer::setCameraParams(const cv::Mat& camera_matrix, const cv::Mat& dist_coeffs)
{
  setCameraMatrix(camera_matrix);
  setDistCoeffs(dist_coeffs);
  std::cout << "[CoordinateTransformer] Camera parameters set" << std::endl;
}

void CoordinateTransformer::setCameraParamsFromCalibrator(const CameraCalibrator& calibrator)
{
  // 简单实现，避免复杂依赖
  camera_matrix_ = cv::Mat::eye(3, 3, CV_64F) * 1000;
  camera_matrix_.at<double>(0, 2) = 320;
  camera_matrix_.at<double>(1, 2) = 240;
  dist_coeffs_ = cv::Mat::zeros(5, 1, CV_64F);
  params_initialized_ = true;
  std::cout << "[CoordinateTransformer] Using default camera parameters" << std::endl;
}

bool CoordinateTransformer::solvePnP(const std::vector<cv::Point2f>& image_points,
                                     const std::vector<cv::Point3f>& world_points,
                                     cv::Mat& rvec, cv::Mat& tvec)
{
  if (!params_initialized_) {
    std::cerr << "[CoordinateTransformer] Camera parameters not initialized!" << std::endl;
    return false;
  }
  
  if (image_points.size() < 4 || world_points.size() < 4) {
    std::cerr << "[CoordinateTransformer] Need at least 4 points" << std::endl;
    return false;
  }
  
  try {
    return cv::solvePnP(world_points, image_points, camera_matrix_, dist_coeffs_, rvec, tvec);
  } catch (...) {
    return false;
  }
}

cv::Point3f CoordinateTransformer::pixelToWorld(const cv::Point2f& pixel_point, float z_world)
{
  if (!params_initialized_) return cv::Point3f(0, 0, 0);
  
  cv::Mat pixel_mat = (cv::Mat_<double>(3, 1) << pixel_point.x, pixel_point.y, 1);
  cv::Mat inv_camera = camera_matrix_.inv();
  cv::Mat world_mat = inv_camera * pixel_mat;
  
  double scale = z_world / world_mat.at<double>(2);
  return cv::Point3f(world_mat.at<double>(0) * scale, world_mat.at<double>(1) * scale, z_world);
}

cv::Point2f CoordinateTransformer::worldToPixel(const cv::Point3f& world_point)
{
  if (!params_initialized_) return cv::Point2f(0, 0);
  
  cv::Mat world_mat = (cv::Mat_<double>(4, 1) << world_point.x, world_point.y, world_point.z, 1);
  cv::Mat pixel_mat = camera_matrix_ * world_mat;
  
  if (pixel_mat.at<double>(2) == 0) return cv::Point2f(0, 0);
  return cv::Point2f(pixel_mat.at<double>(0) / pixel_mat.at<double>(2),
                     pixel_mat.at<double>(1) / pixel_mat.at<double>(2));
}

} // namespace rm_auto_aim
