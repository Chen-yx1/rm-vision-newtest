#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include "armor_detector/pnp_solver.hpp"
#include "armor_detector/armor.hpp"

namespace rm_auto_aim {

PnPSolver::PnPSolver() {
    // 初始化3D世界点
    initWorldPoints();
}

void PnPSolver::initWorldPoints() {
    // 单位：米
    
    // 小装甲板3D点
    constexpr float small_half_y = SMALL_ARMOR_WIDTH / 2.0f;
    constexpr float small_half_z = SMALL_ARMOR_HEIGHT / 2.0f;
    
    // 大装甲板3D点
    constexpr float large_half_y = LARGE_ARMOR_WIDTH / 2.0f;
    constexpr float large_half_z = LARGE_ARMOR_HEIGHT / 2.0f;
    
    // 清除现有点
    small_armor_points_.clear();
    large_armor_points_.clear();
    
    // 模型坐标系：x向前，y向左，z向上
    // 从左上角开始顺时针顺序
    
    // 小装甲板4个角点
    small_armor_points_.push_back(cv::Point3f(0, -small_half_y, small_half_z));   // 左上
    small_armor_points_.push_back(cv::Point3f(0, small_half_y, small_half_z));    // 右上
    small_armor_points_.push_back(cv::Point3f(0, small_half_y, -small_half_z));   // 右下
    small_armor_points_.push_back(cv::Point3f(0, -small_half_y, -small_half_z));  // 左下
    
    // 大装甲板4个角点
    large_armor_points_.push_back(cv::Point3f(0, -large_half_y, large_half_z));   // 左上
    large_armor_points_.push_back(cv::Point3f(0, large_half_y, large_half_z));    // 右上
    large_armor_points_.push_back(cv::Point3f(0, large_half_y, -large_half_z));   // 右下
    large_armor_points_.push_back(cv::Point3f(0, -large_half_y, -large_half_z));  // 左下
}

bool PnPSolver::solvePnP(const Armor& armor, cv::Mat& rvec, cv::Mat& tvec) {
    // 检查相机内参是否已设置
    if (camera_matrix_.empty()) {
        std::cerr << "[ERROR] Camera matrix not set!" << std::endl;
        return false;
    }
    
    // 准备2D图像点
    std::vector<cv::Point2f> image_points;
    
    // 确保装甲板有4个顶点
    if (armor.vertices.size() != 4) {
        std::cerr << "[ERROR] Armor must have exactly 4 vertices!" << std::endl;
        return false;
    }
    
    // 按顺序添加图像点：左上、右上、右下、左下
    image_points.push_back(armor.vertices[0]);  // 左上
    image_points.push_back(armor.vertices[1]);  // 右上
    image_points.push_back(armor.vertices[2]);  // 右下
    image_points.push_back(armor.vertices[3]);  // 左下
    
    // 选择对应的3D点集
    const auto& object_points = (armor.type == ArmorType::SMALL) ? 
                               small_armor_points_ : large_armor_points_;
    
    // 检查2D和3D点数量是否匹配
    if (image_points.size() != object_points.size()) {
        std::cerr << "[ERROR] 2D and 3D point count mismatch!" << std::endl;
        return false;
    }
    
    // 解算PnP
    bool success = cv::solvePnP(
        object_points,          // 3D点
        image_points,           // 2D点
        camera_matrix_,         // 相机内参矩阵
        dist_coeffs_,           // 畸变系数
        rvec,                   // 输出旋转向量
        tvec,                   // 输出平移向量
        false,                  // 不使用初始估计
        cv::SOLVEPNP_IPPE       // 求解方法
    );
    
    return success;
}

float PnPSolver::calculateDistanceToCenter(const cv::Point2f& image_point) {
    if (camera_matrix_.empty()) {
        std::cerr << "[ERROR] Camera matrix not set!" << std::endl;
        return -1.0f;
    }
    
    // 获取图像中心点
    float cx = camera_matrix_.at<double>(0, 2);
    float cy = camera_matrix_.at<double>(1, 2);
    
    // 计算点到中心的欧氏距离
    return cv::norm(image_point - cv::Point2f(cx, cy));
}

} // namespace rm_auto_aim