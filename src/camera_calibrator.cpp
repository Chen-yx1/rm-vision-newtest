#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "armor_detector/camera_calibrator.hpp"

namespace rm_auto_aim {

CameraCalibrator::CameraCalibrator() 
    : calibration_error_(0.0) {
    // 初始化单位矩阵作为默认相机矩阵
    camera_matrix_ = cv::Mat::eye(3, 3, CV_64F);
    dist_coeffs_ = cv::Mat::zeros(5, 1, CV_64F);
}

bool CameraCalibrator::calibrateFromChessboard(const std::string& image_dir,
                                              cv::Size pattern_size,
                                              double square_size,
                                              const std::string& save_path) {
    // 获取目录下所有图像文件
    std::vector<std::string> image_paths;
    DIR* dir = opendir(image_dir.c_str());
    if (!dir) {
        std::cerr << "[ERROR] Cannot open directory: " << image_dir << std::endl;
        return false;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.length() > 4 && 
            (filename.substr(filename.length() - 4) == ".jpg" ||
             filename.substr(filename.length() - 4) == ".png" ||
             filename.substr(filename.length() - 4) == ".bmp")) {
            image_paths.push_back(image_dir + "/" + filename);
        }
    }
    closedir(dir);
    
    if (image_paths.empty()) {
        std::cerr << "[ERROR] No image files found in directory: " << image_dir << std::endl;
        return false;
    }
    
    std::cout << "[INFO] Found " << image_paths.size() << " image files" << std::endl;
    
    // 检测棋盘格角点
    std::vector<std::vector<cv::Point2f>> image_points;
    std::vector<std::vector<cv::Point3f>> object_points;
    
    if (!findChessboardCorners(image_paths, pattern_size, image_points, object_points, square_size)) {
        std::cerr << "[ERROR] Failed to find enough chessboard corners" << std::endl;
        return false;
    }
    
    // 执行相机标定
    std::vector<cv::Mat> rvecs, tvecs;
    calibration_error_ = cv::calibrateCamera(object_points, image_points, 
                                            cv::Size(image_paths.size() > 0 ? 
                                                    cv::imread(image_paths[0]).size() : cv::Size(640, 480)),
                                            camera_matrix_, dist_coeffs_, rvecs, tvecs,
                                            cv::CALIB_FIX_K3 | cv::CALIB_ZERO_TANGENT_DIST);
    
    std::cout << "[INFO] Camera calibration completed!" << std::endl;
    std::cout << "  - Calibration error: " << calibration_error_ << std::endl;
    std::cout << "  - Camera matrix: " << std::endl << camera_matrix_ << std::endl;
    std::cout << "  - Distortion coefficients: " << dist_coeffs_.t() << std::endl;
    
    // 保存标定结果
    return saveCalibration(save_path);
}

bool CameraCalibrator::findChessboardCorners(const std::vector<std::string>& image_paths,
                                            cv::Size pattern_size,
                                            std::vector<std::vector<cv::Point2f>>& image_points,
                                            std::vector<std::vector<cv::Point3f>>& object_points,
                                            double square_size) {
    int success_count = 0;
    
    // 生成世界坐标系中的角点坐标
    std::vector<cv::Point3f> obj_corners;
    for (int i = 0; i < pattern_size.height; i++) {
        for (int j = 0; j < pattern_size.width; j++) {
            obj_corners.push_back(cv::Point3f(j * square_size, i * square_size, 0));
        }
    }
    
    for (size_t i = 0; i < image_paths.size(); i++) {
        cv::Mat image = cv::imread(image_paths[i], cv::IMREAD_GRAYSCALE);
        if (image.empty()) {
            std::cerr << "[WARNING] Cannot read image: " << image_paths[i] << std::endl;
            continue;
        }
        
        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(image, pattern_size, corners,
                                              cv::CALIB_CB_ADAPTIVE_THRESH +
                                              cv::CALIB_CB_NORMALIZE_IMAGE +
                                              cv::CALIB_CB_FAST_CHECK);
        
        if (found) {
            // 亚像素精确化
            cv::cornerSubPix(image, corners, cv::Size(11, 11), cv::Size(-1, -1),
                            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));
            
            image_points.push_back(corners);
            object_points.push_back(obj_corners);
            success_count++;
            
            // 显示检测结果（调试用）
            cv::Mat color_image;
            cv::cvtColor(image, color_image, cv::COLOR_GRAY2BGR);
            cv::drawChessboardCorners(color_image, pattern_size, corners, found);
            cv::imshow("Chessboard Detection", color_image);
            cv::waitKey(100);
        } else {
            std::cout << "[INFO] Chessboard not found in: " << image_paths[i] << std::endl;
        }
    }
    
    cv::destroyWindow("Chessboard Detection");
    std::cout << "[INFO] Successfully processed " << success_count << " out of " 
              << image_paths.size() << " images" << std::endl;
    
    return success_count >= 10;  // 至少需要10张成功的图像
}

bool CameraCalibrator::loadCalibration(const std::string& file_path) {
    cv::FileStorage fs(file_path, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "[ERROR] Cannot open calibration file: " << file_path << std::endl;
        return false;
    }
    
    fs["camera_matrix"] >> camera_matrix_;
    fs["distortion_coefficients"] >> dist_coeffs_;
    fs["calibration_error"] >> calibration_error_;
    
    fs.release();
    
    std::cout << "[INFO] Loaded calibration from: " << file_path << std::endl;
    std::cout << "  - Calibration error: " << calibration_error_ << std::endl;
    
    return true;
}

void CameraCalibrator::setCameraParams(const cv::Mat& camera_matrix, const cv::Mat& dist_coeffs) {
    camera_matrix_ = camera_matrix.clone();
    dist_coeffs_ = dist_coeffs.clone();
}

void CameraCalibrator::undistortImage(const cv::Mat& src, cv::Mat& dst) const {
    if (camera_matrix_.empty() || dist_coeffs_.empty()) {
        std::cerr << "[WARNING] Camera parameters not set for undistortion" << std::endl;
        src.copyTo(dst);
        return;
    }
    
    cv::undistort(src, dst, camera_matrix_, dist_coeffs_);
}

bool CameraCalibrator::saveCalibration(const std::string& file_path) const {
    cv::FileStorage fs(file_path, cv::FileStorage::WRITE);
    if (!fs.isOpened()) {
        std::cerr << "[ERROR] Cannot create calibration file: " << file_path << std::endl;
        return false;
    }
    
    fs << "camera_matrix" << camera_matrix_;
    fs << "distortion_coefficients" << dist_coeffs_;
    fs << "calibration_error" << calibration_error_;
    fs << "calibration_date" << cv::getTickCount() / cv::getTickFrequency();
    
    fs.release();
    
    std::cout << "[INFO] Calibration saved to: " << file_path << std::endl;
    return true;
}

void CameraCalibrator::generateDummyCameraParams(cv::Mat& camera_matrix, cv::Mat& dist_coeffs,
                                                int image_width, int image_height) {
    // 生成虚拟相机参数（用于测试）
    camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    camera_matrix.at<double>(0, 0) = 800.0;  // fx
    camera_matrix.at<double>(1, 1) = 800.0;  // fy
    camera_matrix.at<double>(0, 2) = image_width / 2.0;   // cx
    camera_matrix.at<double>(1, 2) = image_height / 2.0;  // cy
    
    dist_coeffs = cv::Mat::zeros(5, 1, CV_64F);
    dist_coeffs.at<double>(0) = 0.1;   // k1
    dist_coeffs.at<double>(1) = -0.1;  // k2
    dist_coeffs.at<double>(2) = 0.001; // p1
    dist_coeffs.at<double>(3) = 0.001; // p2
    dist_coeffs.at<double>(4) = 0.0;   // k3
}

} // namespace rm_auto_aim