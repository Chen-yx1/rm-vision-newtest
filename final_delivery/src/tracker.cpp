#include <cmath>
#include <algorithm>
#include <iostream>
#include "armor_detector/tracker.hpp"

namespace rm_auto_aim {

Tracker::Tracker() {
    kf_ = std::make_unique<KalmanFilter>();
    reset();
}

void Tracker::reset() {
    state_ = LOST;
    is_tracking_ = false;
    tracked_armor_ = nullptr;
    detect_count_ = 0;
    lost_count_ = 0;
}

void Tracker::init(const Armor& armor) {
    if (!armor.isValid()) {
        reset();
        return;
    }
    
    // 初始化卡尔曼滤波器
    kf_->init(armor.center);
    
    tracked_armor_ = &armor;
    detect_count_ = 1;
    lost_count_ = 0;
    state_ = DETECTING;
    is_tracking_ = false;
    
    std::cout << "[TRACKER] Initialized at position: " 
              << armor.center.x << ", " << armor.center.y << std::endl;
}

const Armor* Tracker::selectBestMatch(const std::vector<Armor>& armors) {
    if (armors.empty() || !is_tracking_) {
        return nullptr;
    }
    
    // 如果有跟踪的装甲板，选择匹配度最高的
    const Armor* best_match = nullptr;
    float best_score = std::numeric_limits<float>::max();
    
    for (const auto& armor : armors) {
        if (!armor.isValid()) continue;
        
        float score = calculateMatchScore(armor);
        if (score < best_score) {
            best_score = score;
            best_match = &armor;
        }
    }
    
    // 检查是否满足匹配条件
    if (best_match && best_score < max_match_distance_) {
        return best_match;
    }
    
    return nullptr;
}

float Tracker::calculateMatchScore(const Armor& armor) {
    if (!is_tracking_ || !tracked_armor_) {
        return std::numeric_limits<float>::max();
    }
    
    // 计算位置差异
    cv::Point2f pred_pos = predicted_position_;
    cv::Point2f curr_pos = armor.center;
    
    float distance = cv::norm(pred_pos - curr_pos);
    
    // 计算尺寸差异
    float width_ratio = std::max(armor.vertices[1].x - armor.vertices[0].x, 
                                tracked_armor_->vertices[1].x - tracked_armor_->vertices[0].x) /
                       std::min(armor.vertices[1].x - armor.vertices[0].x, 
                                tracked_armor_->vertices[1].x - tracked_armor_->vertices[0].x);
    
    float height_ratio = std::max(armor.vertices[2].y - armor.vertices[1].y,
                                 tracked_armor_->vertices[2].y - tracked_armor_->vertices[1].y) /
                        std::min(armor.vertices[2].y - armor.vertices[1].y,
                                 tracked_armor_->vertices[2].y - tracked_armor_->vertices[1].y);
    
    float size_penalty = std::max(width_ratio, height_ratio);
    
    // 综合评分（位置差异为主，尺寸差异为辅）
    return distance * (1.0f + 0.1f * (size_penalty - 1.0f));
}

void Tracker::update(const std::vector<Armor>& armors) {
    switch (state_) {
        case LOST:
            if (!armors.empty()) {
                // 选择距离图像中心最近的装甲板开始跟踪
                const Armor* closest_armor = &armors[0];
                float min_distance_to_center = std::numeric_limits<float>::max();
                
                for (const auto& armor : armors) {
                    if (!armor.isValid()) continue;
                    
                    float distance = cv::norm(armor.center - cv::Point2f(640, 360)); // 假设图像中心
                    if (distance < min_distance_to_center) {
                        min_distance_to_center = distance;
                        closest_armor = &armor;
                    }
                }
                
                init(*closest_armor);
            }
            break;
            
        case DETECTING:
            if (!armors.empty()) {
                const Armor* match = selectBestMatch(armors);
                if (match) {
                    detect_count_++;
                    tracked_armor_ = match;
                    kf_->update(match->center);
                    
                    if (detect_count_ >= tracking_thres_) {
                        state_ = TRACKING;
                        is_tracking_ = true;
                        std::cout << "[TRACKER] Now TRACKING target" << std::endl;
                    }
                } else {
                    detect_count_ = 0;
                    state_ = LOST;
                }
            } else {
                detect_count_ = 0;
                state_ = LOST;
            }
            break;
            
        case TRACKING:
            // 先进行预测
            predicted_position_ = kf_->predict();
            
            if (!armors.empty()) {
                const Armor* match = selectBestMatch(armors);
                if (match) {
                    // 找到匹配，更新滤波器
                    tracked_armor_ = match;
                    kf_->update(match->center);
                    lost_count_ = 0;
                    
                    // 更新预测位置
                    predicted_position_ = kf_->getPrediction();
                } else {
                    // 未找到匹配，临时丢失
                    lost_count_++;
                    if (lost_count_ >= lost_thres_) {
                        state_ = TEMP_LOST;
                        std::cout << "[TRACKER] Target TEMPORARILY LOST" << std::endl;
                    }
                }
            } else {
                // 没有检测到任何装甲板
                lost_count_++;
                if (lost_count_ >= lost_thres_) {
                    state_ = TEMP_LOST;
                    std::cout << "[TRACKER] Target TEMPORARILY LOST" << std::endl;
                }
            }
            break;
            
        case TEMP_LOST:
            // 仍然进行预测
            predicted_position_ = kf_->predict();
            
            if (!armors.empty()) {
                const Armor* match = selectBestMatch(armors);
                if (match) {
                    // 重新找到目标，恢复跟踪
                    tracked_armor_ = match;
                    kf_->update(match->center);
                    lost_count_ = 0;
                    state_ = TRACKING;
                    std::cout << "[TRACKER] Target found again, back to TRACKING" << std::endl;
                } else {
                    lost_count_++;
                    if (lost_count_ >= lost_thres_ * 2) {
                        // 长时间未找到，重置跟踪器
                        reset();
                        std::cout << "[TRACKER] Target LOST, resetting tracker" << std::endl;
                    }
                }
            } else {
                lost_count_++;
                if (lost_count_ >= lost_thres_ * 2) {
                    reset();
                    std::cout << "[TRACKER] Target LOST, resetting tracker" << std::endl;
                }
            }
            break;
    }
}

} // namespace rm_auto_aim