#include "armor_detector/detector.hpp"
#include <iostream>
#include <cmath>

namespace rm_auto_aim {

Detector::Detector(const DetectorParams& params) : params_(params) {
    std::cout << "[DETECTOR] 检测器初始化完成" << std::endl;
    std::cout << "[DETECTOR] 检测颜色: " << (params.detect_color == ColorMode::RED ? "红色" : "蓝色") << std::endl;
}

cv::Mat Detector::preprocessImage(const cv::Mat& rgb_img) const {
    if (rgb_img.empty()) {
        std::cerr << "[ERROR] 输入图像为空!" << std::endl;
        return cv::Mat();
    }
    
    cv::Mat hsv, mask_red1, mask_red2, mask_red, mask_blue, mask_result;
    
    // 转换为HSV颜色空间
    cv::cvtColor(rgb_img, hsv, cv::COLOR_BGR2HSV);
    
    if (params_.detect_color == ColorMode::RED) {
        // 红色有两个范围: 0-10 和 160-180
        cv::inRange(hsv, 
                   cv::Scalar(0, params_.hsv_red.s_min, params_.hsv_red.v_min),
                   cv::Scalar(params_.hsv_red.h_max, params_.hsv_red.s_max, params_.hsv_red.v_max),
                   mask_red1);
        
        cv::inRange(hsv,
                   cv::Scalar(160, params_.hsv_red.s_min, params_.hsv_red.v_min),
                   cv::Scalar(180, params_.hsv_red.s_max, params_.hsv_red.v_max),
                   mask_red2);
        
        // 合并两个红色范围
        cv::bitwise_or(mask_red1, mask_red2, mask_red);
        mask_result = mask_red;
    } else {
        // 蓝色范围
        cv::inRange(hsv,
                   cv::Scalar(params_.hsv_blue.h_min, params_.hsv_blue.s_min, params_.hsv_blue.v_min),
                   cv::Scalar(params_.hsv_blue.h_max, params_.hsv_blue.s_max, params_.hsv_blue.v_max),
                   mask_blue);
        mask_result = mask_blue;
    }
    
    // 形态学操作：去除噪声，连接区域
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    
    // 闭操作：连接相邻区域
    cv::morphologyEx(mask_result, mask_result, cv::MORPH_CLOSE, kernel);
    
    // 开操作：去除小噪声
    cv::morphologyEx(mask_result, mask_result, cv::MORPH_OPEN, kernel);
    
    return mask_result;
}

std::vector<Light> Detector::findLights(const cv::Mat& binary_img) const {
    std::vector<Light> lights;
    
    if (binary_img.empty()) {
        std::cerr << "[ERROR] 二值图像为空!" << std::endl;
        return lights;
    }
    
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
    // 查找轮廓
    cv::findContours(binary_img, contours, hierarchy, 
                     cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    std::cout << "[DETECTOR] 找到 " << contours.size() << " 个轮廓" << std::endl;
    
    for (size_t i = 0; i < contours.size(); i++) {
        // 跳过太小的轮廓
        if (contours[i].size() < 5) {
            continue;
        }
        
        // 获取最小外接矩形
        cv::RotatedRect rect = cv::minAreaRect(contours[i]);
        
        // 创建灯条对象
        Light light(rect);
        
        // 计算长宽比
        float ratio = light.width / (light.length + 1e-5f);
        
        // 筛选条件
        bool ratio_ok = (ratio > params_.light.min_ratio) && 
                       (ratio < params_.light.max_ratio);
        bool angle_ok = light.tilt_angle < params_.light.max_angle;
        
        if (ratio_ok && angle_ok) {
            light.color = static_cast<int>(params_.detect_color);
            lights.push_back(light);
            
            std::cout << "[DETECTOR] 灯条 " << lights.size() << ": "
                      << "长宽比=" << ratio << ", "
                      << "角度=" << light.tilt_angle << "°, "
                      << "中心=(" << light.center.x << "," << light.center.y << ")"
                      << std::endl;
        }
    }
    
    std::cout << "[DETECTOR] 筛选后剩余 " << lights.size() << " 个灯条" << std::endl;
    return lights;
}

} // namespace rm_auto_aim