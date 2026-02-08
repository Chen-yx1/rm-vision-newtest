#include <opencv2/opencv.hpp>
#include "armor_detector/armor.hpp"
#include "armor_detector/detector.hpp"

namespace rm_auto_aim {

// 构造函数 - 需要修正为使用指针
Armor::Armor(const Light& left_light, const Light& right_light) 
    : left_light(&left_light), right_light(&right_light) {
    // 计算中心点
    center = (left_light.center + right_light.center) * 0.5f;
    // 更新顶点
    updateVertices();
}

// 更新顶点
void Armor::updateVertices() {
    vertices.clear();
    
    if (!left_light || !right_light) return;
    
    // 左灯条的顶部和底部
    cv::Point2f left_top = left_light->top;
    cv::Point2f left_bottom = left_light->bottom;
    
    // 右灯条的顶部和底部
    cv::Point2f right_top = right_light->top;
    cv::Point2f right_bottom = right_light->bottom;
    
    // 确保顶点顺序：左上 -> 右上 -> 右下 -> 左下
    // 通过y坐标确定上下
    if (left_top.y > left_bottom.y) {
        std::swap(left_top, left_bottom);
    }
    if (right_top.y > right_bottom.y) {
        std::swap(right_top, right_bottom);
    }
    
    // 通过x坐标确定左右
    if (left_top.x > right_top.x) {
        std::swap(left_top, right_top);
        std::swap(left_bottom, right_bottom);
    }
    
    // 添加顶点
    vertices.push_back(left_top);
    vertices.push_back(right_top);
    vertices.push_back(right_bottom);
    vertices.push_back(left_bottom);
}

// 绘制装甲板 - 确保这里有 const !!!
void Armor::draw(cv::Mat& img, const cv::Scalar& color, int thickness) const {
    if (vertices.size() == 4) {
        // 绘制四边形
        for (size_t i = 0; i < 4; ++i) {
            cv::line(img, vertices[i], vertices[(i + 1) % 4], color, thickness);
        }
        
        // 绘制中心点
        cv::circle(img, center, 3, cv::Scalar(255, 255, 0), -1);
        
        // 绘制装甲板类型文字
        std::string type_str;
        if (type == ArmorType::SMALL) {
            type_str = "SMALL";
        } else if (type == ArmorType::LARGE) {
            type_str = "LARGE";
        } else {
            type_str = "INVALID";
        }
        
        cv::putText(img, type_str, center + cv::Point2f(-20, -20), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
    }
}

// 计算顶点（内部使用）
void Armor::calculateVertices() {
    // 这个函数已经被updateVertices()替代
}

} // namespace rm_auto_aim