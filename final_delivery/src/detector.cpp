#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "armor_detector/detector.hpp"

namespace rm_auto_aim {

using Clock = std::chrono::high_resolution_clock;

Detector::Detector(const DetectorParams& params) : params_(params) {
    std::cout << "[INIT] Detector initialized with armor matching" << std::endl;
}

std::vector<Armor> Detector::detect(const cv::Mat& rgb_img) {
    auto start_time = Clock::now();
    debug_info_ = DebugInfo();
    
    // 1. 预处理
    binary_img_ = preprocess(rgb_img);
    
    // 2. 查找灯条
    lights_ = findLights(rgb_img, binary_img_);
    
    // 3. 匹配装甲板
    armors_ = matchLights(lights_);
    
    auto end_time = Clock::now();
    debug_info_.process_time_ms = 
        std::chrono::duration<double, std::milli>(end_time - start_time).count();
    debug_info_.armors_found = armors_.size();
    
    std::cout << "[DEBUG] Frame: " 
              << debug_info_.contours_found << " contours -> " 
              << debug_info_.lights_found << " lights -> " 
              << debug_info_.target_color_lights << " target lights -> "
              << debug_info_.armors_found << " armors"
              << " (" << std::fixed << std::setprecision(2) 
              << debug_info_.process_time_ms << " ms)" << std::endl;
    
    return armors_;
}

cv::Mat Detector::preprocess(const cv::Mat& rgb_img) {
    if (rgb_img.empty() || rgb_img.channels() != 3) {
        return cv::Mat();
    }
    
    cv::Mat hsv_img;
    cv::cvtColor(rgb_img, hsv_img, cv::COLOR_BGR2HSV);
    
    cv::Mat color_mask;
    
    if (params_.detect_color == RED) {
        cv::Mat mask1, mask2;
        cv::inRange(hsv_img, 
                   cv::Scalar(params_.hsv_red.h1_min, params_.hsv_red.s_min, params_.hsv_red.v_min),
                   cv::Scalar(params_.hsv_red.h1_max, params_.hsv_red.s_max, params_.hsv_red.v_max),
                   mask1);
        
        cv::inRange(hsv_img,
                   cv::Scalar(params_.hsv_red.h2_min, params_.hsv_red.s_min, params_.hsv_red.v_min),
                   cv::Scalar(params_.hsv_red.h2_max, params_.hsv_red.s_max, params_.hsv_red.v_max),
                   mask2);
        
        cv::bitwise_or(mask1, mask2, color_mask);
    } else {
        cv::inRange(hsv_img,
                   cv::Scalar(100, params_.hsv_red.s_min, params_.hsv_red.v_min),
                   cv::Scalar(130, params_.hsv_red.s_max, params_.hsv_red.v_max),
                   color_mask);
    }
    
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(color_mask, color_mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(color_mask, color_mask, cv::MORPH_OPEN, kernel);
    
    return color_mask;
}

std::vector<Light> Detector::findLights(const cv::Mat& rgb_img, const cv::Mat& binary_img) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::findContours(binary_img.clone(), contours, hierarchy, 
                     cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    debug_info_.contours_found = contours.size();
    std::vector<Light> valid_lights;
    
    for (size_t i = 0; i < contours.size(); ++i) {
        if (contours[i].size() < 5) continue;
        
        double area = cv::contourArea(contours[i]);
        if (area < params_.light.min_area) continue;
        
        cv::RotatedRect rotated_rect = cv::minAreaRect(contours[i]);
        
        Light light;
        light.rect = rotated_rect;
        light.center = rotated_rect.center;
        light.width = std::min(rotated_rect.size.width, rotated_rect.size.height);
        light.length = std::max(rotated_rect.size.width, rotated_rect.size.height);
        
        // 计算灯条顶部和底部
        cv::Point2f vertices[4];
        rotated_rect.points(vertices);
        
        // 找到y坐标最小和最大的点作为顶部和底部
        int top_idx = 0, bottom_idx = 0;
        for (int j = 1; j < 4; j++) {
            if (vertices[j].y < vertices[top_idx].y) top_idx = j;
            if (vertices[j].y > vertices[bottom_idx].y) bottom_idx = j;
        }
        light.top = vertices[top_idx];
        light.bottom = vertices[bottom_idx];
        
        light.angle = rotated_rect.angle;
        if (light.angle < -45.0) light.angle += 90.0;
        light.angle = std::abs(light.angle);
        
        if (isValidLight(light)) {
            debug_info_.lights_found++;
            light.color = determineColor(rgb_img, light);
            
            if (light.color == params_.detect_color) {
                valid_lights.push_back(light);
                debug_info_.target_color_lights++;
            }
        }
    }
    
    return valid_lights;
}

bool Detector::isValidLight(const Light& light) {
    float ratio = light.width / (light.length + 1e-5f);
    
    if (ratio < params_.light.min_ratio || ratio > params_.light.max_ratio) {
        return false;
    }
    
    if (light.angle > params_.light.max_angle) {
        return false;
    }
    
    return true;
}

int Detector::determineColor(const cv::Mat& rgb_img, const Light& light) {
    cv::Rect bbox = light.rect.boundingRect();
    bbox.x = std::max(0, bbox.x);
    bbox.y = std::max(0, bbox.y);
    bbox.width = std::min(bbox.width, rgb_img.cols - bbox.x);
    bbox.height = std::min(bbox.height, rgb_img.rows - bbox.y);
    
    if (bbox.width <= 0 || bbox.height <= 0) {
        return params_.detect_color;
    }
    
    cv::Mat roi = rgb_img(bbox);
    
    // 获取旋转矩形的四个顶点
    cv::Point2f vertices[4];
    light.rect.points(vertices);
    
    // 将顶点转换为轮廓点向量
    std::vector<cv::Point2f> contour;
    for (int i = 0; i < 4; i++) {
        contour.push_back(vertices[i]);
    }
    
    long long sum_r = 0, sum_b = 0;
    int pixel_count = 0;
    
    for (int y = 0; y < roi.rows; ++y) {
        const cv::Vec3b* img_ptr = roi.ptr<cv::Vec3b>(y);
        for (int x = 0; x < roi.cols; ++x) {
            // 检查点是否在灯条轮廓内
            cv::Point2f roi_point(x + bbox.x, y + bbox.y);
            if (cv::pointPolygonTest(contour, roi_point, false) >= 0) {
                sum_b += img_ptr[x][0];  // B通道
                sum_r += img_ptr[x][2];  // R通道
                pixel_count++;
            }
        }
    }
    
    if (pixel_count == 0) return params_.detect_color;
    return (sum_r > sum_b) ? RED : BLUE;
}

std::vector<Armor> Detector::matchLights(const std::vector<Light>& lights) {
    std::vector<Armor> armors;
    
    // 只考虑目标颜色的灯条
    std::vector<Light> target_lights;
    for (const auto& light : lights) {
        if (light.color == params_.detect_color) {
            target_lights.push_back(light);
        }
    }
    
    if (target_lights.size() < 2) {
        return armors;
    }
    
    // 对所有灯条进行配对
    for (size_t i = 0; i < target_lights.size(); ++i) {
        for (size_t j = i + 1; j < target_lights.size(); ++j) {
            const Light& light1 = target_lights[i];
            const Light& light2 = target_lights[j];
            
            // 检查配对是否有效
            ArmorType type = isArmor(light1, light2);
            if (type != ArmorType::INVALID) {
                // 检查区域内是否有其他灯条
                if (!containLight(light1, light2, target_lights)) {
                    Armor armor(light1, light2);
                    armor.type = type;
                    armors.push_back(armor);
                }
            }
        }
    }
    
    return armors;
}

ArmorType Detector::isArmor(const Light& light1, const Light& light2) {
    // 1. 计算灯条长度比
    float length_ratio = std::min(light1.length, light2.length) / 
                        (std::max(light1.length, light2.length) + 1e-5f);
    if (length_ratio < params_.armor.min_light_ratio) {
        return ArmorType::INVALID;
    }
    
    // 2. 计算中心距离（单位：平均灯条长度）
    float avg_length = (light1.length + light2.length) * 0.5f;
    float center_distance = cv::norm(light1.center - light2.center) / avg_length;
    
    // 3. 判断装甲板类型
    ArmorType type = ArmorType::INVALID;
    if (center_distance >= params_.armor.min_small_distance &&
        center_distance <= params_.armor.max_small_distance) {
        type = ArmorType::SMALL;
    } else if (center_distance >= params_.armor.min_large_distance &&
               center_distance <= params_.armor.max_large_distance) {
        type = ArmorType::LARGE;
    }
    
    if (type == ArmorType::INVALID) {
        return ArmorType::INVALID;
    }
    
    // 4. 检查角度差
    float angle_diff = std::abs(light1.angle - light2.angle);
    if (angle_diff > params_.armor.max_angle_diff) {
        return ArmorType::INVALID;
    }
    
    // 5. 检查垂直比例（防止竖立的灯条配对）
    float vertical_diff = std::abs(light1.center.y - light2.center.y);
    float horizontal_diff = std::abs(light1.center.x - light2.center.x);
    if (vertical_diff > horizontal_diff * params_.armor.max_vertical_ratio) {
        return ArmorType::INVALID;
    }
    
    // 6. 检查宽高比
    float width = cv::norm(light1.center - light2.center);
    float height = avg_length;
    float aspect_ratio = width / (height + 1e-5f);
    if (aspect_ratio < params_.armor.min_aspect_ratio ||
        aspect_ratio > params_.armor.max_aspect_ratio) {
        return ArmorType::INVALID;
    }
    
    return type;
}

bool Detector::containLight(const Light& light1, const Light& light2, 
                           const std::vector<Light>& lights) {
    // 获取两个灯条形成的包围矩形
    std::vector<cv::Point2f> points = {
        light1.top, light1.bottom, light2.top, light2.bottom
    };
    cv::Rect bbox = cv::boundingRect(points);
    
    // 扩展边界
    bbox.x -= 5;
    bbox.y -= 5;
    bbox.width += 10;
    bbox.height += 10;
    
    // 检查其他灯条是否在区域内
    for (const auto& light : lights) {
        if (&light == &light1 || &light == &light2) {
            continue;
        }
        
        if (bbox.contains(light.center) ||
            bbox.contains(light.top) ||
            bbox.contains(light.bottom)) {
            return true;
        }
    }
    
    return false;
}

} // namespace rm_auto_aim