#!/bin/bash
echo "=== 修复CameraCalibrator问题 ==="

echo "1. 检查头文件包含..."
echo "coordinate_transformer.hpp内容："
head -30 include/armor_detector/coordinate_transformer.hpp

echo ""
echo "2. 修复coordinate_transformer.hpp..."
cat > include/armor_detector/coordinate_transformer.hpp << 'HEADER_EOF'
#pragma once

#include <opencv2/opencv.hpp>
#include "armor_detector/camera_calibrator.hpp"

namespace rm_auto_aim
{
class CoordinateTransformer
{
public:
  CoordinateTransformer();
  
  // 设置相机参数
  void setCameraMatrix(const cv::Mat& matrix);
  void setDistCoeffs(const cv::Mat& coeffs);
  void setCameraParams(const cv::Mat& camera_matrix, const cv::Mat& dist_coeffs);
  void setCameraParamsFromCalibrator(const CameraCalibrator& calibrator);
  
  // 坐标转换方法
  bool solvePnP(const std::vector<cv::Point2f>& image_points,
                const std::vector<cv::Point3f>& world_points,
                cv::Mat& rvec, cv::Mat& tvec);
                
  cv::Point3f pixelToWorld(const cv::Point2f& pixel_point, float z_world = 0);
  cv::Point2f worldToPixel(const cv::Point3f& world_point);
  
  // 获取参数
  cv::Mat getCameraMatrix() const { return camera_matrix_; }
  cv::Mat getDistCoeffs() const { return dist_coeffs_; }
  
private:
  cv::Mat camera_matrix_;
  cv::Mat dist_coeffs_;
  
  bool params_initialized_ = false;
};

} // namespace rm_auto_aim
HEADER_EOF

echo "3. 修复coordinate_transformer.cpp..."
cat > src/coordinate_transformer.cpp << 'CPP_EOF'
#include "armor_detector/coordinate_transformer.hpp"
#include <iostream>

namespace rm_auto_aim
{

CoordinateTransformer::CoordinateTransformer()
{
  // 默认相机参数（可以后续通过标定更新）
  camera_matrix_ = (cv::Mat_<double>(3, 3) << 
    1000.0, 0.0, 320.0,
    0.0, 1000.0, 240.0,
    0.0, 0.0, 1.0);
  
  dist_coeffs_ = cv::Mat::zeros(5, 1, CV_64F);
  params_initialized_ = true;
}

void CoordinateTransformer::setCameraMatrix(const cv::Mat& matrix)
{
  if (matrix.empty()) return;
  matrix.copyTo(camera_matrix_);
  params_initialized_ = true;
}

void CoordinateTransformer::setDistCoeffs(const cv::Mat& coeffs)
{
  if (coeffs.empty()) return;
  coeffs.copyTo(dist_coeffs_);
}

void CoordinateTransformer::setCameraParams(const cv::Mat& camera_matrix, const cv::Mat& dist_coeffs)
{
  setCameraMatrix(camera_matrix);
  setDistCoeffs(dist_coeffs);
  std::cout << "[CoordinateTransformer] Camera parameters set" << std::endl;
}

void CoordinateTransformer::setCameraParamsFromCalibrator(const CameraCalibrator& calibrator)
{
  camera_matrix_ = calibrator.getCameraMatrix().clone();
  dist_coeffs_ = calibrator.getDistCoeffs().clone();
  params_initialized_ = true;
  std::cout << "[CoordinateTransformer] Camera parameters loaded from calibrator" << std::endl;
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
    std::cerr << "[CoordinateTransformer] Need at least 4 points for PnP" << std::endl;
    return false;
  }
  
  try {
    cv::solvePnP(world_points, image_points, camera_matrix_, dist_coeffs_, rvec, tvec, false, cv::SOLVEPNP_IPPE);
    return true;
  } catch (const cv::Exception& e) {
    std::cerr << "[CoordinateTransformer] PnP solve error: " << e.what() << std::endl;
    return false;
  }
}

cv::Point3f CoordinateTransformer::pixelToWorld(const cv::Point2f& pixel_point, float z_world)
{
  if (!params_initialized_) return cv::Point3f(0, 0, 0);
  
  // 简单投影（假设平面z=z_world）
  cv::Mat pixel_mat = (cv::Mat_<double>(3, 1) << pixel_point.x, pixel_point.y, 1);
  cv::Mat inv_camera = camera_matrix_.inv();
  cv::Mat world_mat = inv_camera * pixel_mat;
  
  float x = world_mat.at<double>(0) * z_world;
  float y = world_mat.at<double>(1) * z_world;
  
  return cv::Point3f(x, y, z_world);
}

cv::Point2f CoordinateTransformer::worldToPixel(const cv::Point3f& world_point)
{
  if (!params_initialized_) return cv::Point2f(0, 0);
  
  cv::Mat world_mat = (cv::Mat_<double>(3, 1) << world_point.x, world_point.y, world_point.z);
  cv::Mat pixel_mat = camera_matrix_ * world_mat;
  
  float u = pixel_mat.at<double>(0) / pixel_mat.at<double>(2);
  float v = pixel_mat.at<double>(1) / pixel_mat.at<double>(2);
  
  return cv::Point2f(u, v);
}

} // namespace rm_auto_aim
CPP_EOF

echo "4. 检查camera_calibrator.hpp是否存在..."
if [ -f "include/armor_detector/camera_calibrator.hpp" ]; then
  echo "✅ camera_calibrator.hpp存在"
  echo "检查内容："
  head -20 include/armor_detector/camera_calibrator.hpp
else
  echo "❌ camera_calibrator.hpp不存在，创建基本版本..."
  cat > include/armor_detector/camera_calibrator.hpp << 'CALIB_EOF'
#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace rm_auto_aim
{
class CameraCalibrator
{
public:
  CameraCalibrator();
  
  bool calibrateFromImages(const std::vector<std::string>& image_paths,
                          int board_width, int board_height, float square_size);
  
  bool calibrateFromVideo(const std::string& video_path,
                         int board_width, int board_height, float square_size,
                         int num_frames = 20);
  
  bool loadCalibration(const std::string& filepath);
  bool saveCalibration(const std::string& filepath) const;
  
  cv::Mat getCameraMatrix() const { return camera_matrix_; }
  cv::Mat getDistCoeffs() const { return dist_coeffs_; }
  double getReprojectionError() const { return reprojection_error_; }
  
  bool isCalibrated() const { return calibrated_; }
  
private:
  cv::Mat camera_matrix_;
  cv::Mat dist_coeffs_;
  double reprojection_error_ = 0.0;
  bool calibrated_ = false;
  
  std::vector<std::vector<cv::Point2f>> image_points_;
  std::vector<std::vector<cv::Point3f>> object_points_;
};

} // namespace rm_auto_aim
CALIB_EOF
fi

echo "=== 修复完成！ ==="
