#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "armor_detector/detector.hpp"
#include "armor_detector/armor.hpp"
#include "armor_detector/tracker.hpp"  // 新增

rm_auto_aim::DetectorParams g_params;
rm_auto_aim::Detector* g_detector_ptr = nullptr;
rm_auto_aim::Tracker g_tracker;  // 新增：跟踪器实例

void onTrackbarChange(int, void*) {
    if (g_detector_ptr) {
        *g_detector_ptr = rm_auto_aim::Detector(g_params);
    }
}

void drawDebugInfo(cv::Mat& img, const rm_auto_aim::Detector::DebugInfo& info, 
                   const rm_auto_aim::DetectorParams& params,
                   const rm_auto_aim::Tracker& tracker) {  // 修改
    int y_offset = 25;
    int line_height = 25;
    
    cv::rectangle(img, cv::Point(5, 5), cv::Point(400, 200), cv::Scalar(0, 0, 0), cv::FILLED);
    cv::rectangle(img, cv::Point(5, 5), cv::Point(400, 200), cv::Scalar(255, 255, 255), 1);
    
    cv::putText(img, "RM Vision 2.2.1.3 - Armor Detection + Tracking", 
                cv::Point(10, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.6, 
                cv::Scalar(0, 255, 255), 1);
    y_offset += line_height;
    
    std::string result_text = "Armors: " + std::to_string(info.armors_found) +
                             " | Lights: " + std::to_string(info.target_color_lights);
    cv::putText(img, result_text, cv::Point(10, y_offset), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 1);
    y_offset += line_height;
    
    std::string time_text = "Time: " + std::to_string(info.process_time_ms) + " ms";
    cv::putText(img, time_text, cv::Point(10, y_offset), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(200, 200, 200), 1);
    y_offset += line_height;
    
    std::string color_text = "Color: " + std::string(params.detect_color == rm_auto_aim::RED ? "RED" : "BLUE");
    cv::putText(img, color_text, cv::Point(10, y_offset), 
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 200), 1);
    y_offset += line_height - 5;
    
    // 新增：跟踪器状态显示
    std::string tracker_state;
    switch (tracker.getState()) {
        case rm_auto_aim::Tracker::LOST:
            tracker_state = "LOST";
            break;
        case rm_auto_aim::Tracker::DETECTING:
            tracker_state = "DETECTING";
            break;
        case rm_auto_aim::Tracker::TRACKING:
            tracker_state = "TRACKING";
            break;
        case rm_auto_aim::Tracker::TEMP_LOST:
            tracker_state = "TEMP_LOST";
            break;
    }
    
    std::string tracker_text = "Tracker: " + tracker_state;
    cv::Scalar tracker_color;
    if (tracker.getState() == rm_auto_aim::Tracker::TRACKING) {
        tracker_color = cv::Scalar(0, 255, 0);
    } else if (tracker.getState() == rm_auto_aim::Tracker::TEMP_LOST) {
        tracker_color = cv::Scalar(0, 165, 255);
    } else {
        tracker_color = cv::Scalar(200, 200, 200);
    }
    
    cv::putText(img, tracker_text, cv::Point(10, y_offset), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, tracker_color, 1);
    y_offset += line_height - 5;
    
    std::string param_text = "V_min: " + std::to_string(params.hsv_red.v_min) + 
                           " | S_min: " + std::to_string(params.hsv_red.s_min);
    cv::putText(img, param_text, cv::Point(10, y_offset), 
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 200), 1);
    y_offset += line_height - 5;
    
    cv::putText(img, "q=quit, s=save, space=pause, +/-=V_min, c=toggle color, r=reset", 
                cv::Point(10, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.4, 
                cv::Scalar(150, 150, 255), 1);
}

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "RoboMaster Vision Assessment 2.2.1.3" << std::endl;
    std::cout << "Task: Armor Detection with Kalman Filter Tracking" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::string video_source = "test_video.mp4";
    bool use_camera = false;
    
    if (argc >= 2) {
        video_source = argv[1];
        if (video_source == "camera" || video_source == "0") {
            use_camera = true;
            video_source = "0";
        }
    }
    
    cv::VideoCapture cap;
    if (use_camera) {
        cap.open(0);
        if (!cap.isOpened()) {
            std::cerr << "[ERROR] Cannot open camera!" << std::endl;
            return -1;
        }
        std::cout << "[INFO] Using camera" << std::endl;
    } else {
        cap.open(video_source);
        if (!cap.isOpened()) {
            std::cerr << "[ERROR] Cannot open video file: " << video_source << std::endl;
            cap.open(0);
            use_camera = true;
            
            if (!cap.isOpened()) {
                std::cerr << "[ERROR] Cannot open camera either!" << std::endl;
                return -1;
            }
        } else {
            std::cout << "[INFO] Using video file: " << video_source << std::endl;
        }
    }
    
    g_params.detect_color = rm_auto_aim::RED;
    g_params.hsv_red.v_min = 100;
    g_params.hsv_red.v_max = 255;
    g_params.hsv_red.s_min = 100;
    g_params.hsv_red.s_max = 255;
    
    rm_auto_aim::Detector detector(g_params);
    g_detector_ptr = &detector;
    
    cv::namedWindow("Armor Detection + Tracking", cv::WINDOW_NORMAL);
    cv::resizeWindow("Armor Detection + Tracking", 800, 600);
    cv::namedWindow("Binary Mask", cv::WINDOW_NORMAL);
    cv::resizeWindow("Binary Mask", 400, 300);
    
    cv::createTrackbar("V_min", "Armor Detection + Tracking", &g_params.hsv_red.v_min, 
                       255, onTrackbarChange);
    cv::createTrackbar("S_min", "Armor Detection + Tracking", &g_params.hsv_red.s_min, 
                       255, onTrackbarChange);
    
    std::cout << "\n[CONTROLS]" << std::endl;
    std::cout << "  q / ESC   - Quit" << std::endl;
    std::cout << "  s         - Save current frame" << std::endl;
    std::cout << "  space     - Pause/Resume" << std::endl;
    std::cout << "  +/-       - Adjust V_min" << std::endl;
    std::cout << "  c         - Toggle color (RED/BLUE)" << std::endl;
    std::cout << "  r         - Reset parameters and tracker" << std::endl;
    std::cout << "  t         - Reset tracker only" << std::endl;
    std::cout << "\n[TRACKER STATES]" << std::endl;
    std::cout << "  LOST      - No target" << std::endl;
    std::cout << "  DETECTING - Detected, confirming target" << std::endl;
    std::cout << "  TRACKING  - Actively tracking target" << std::endl;
    std::cout << "  TEMP_LOST - Temporarily lost, predicting position" << std::endl;
    std::cout << "\n[STARTING ARMOR DETECTION WITH TRACKING]" << std::endl;
    
    cv::Mat frame;
    int frame_count = 0;
    bool paused = false;
    
    while (true) {
        if (!paused) {
            if (!cap.read(frame)) {
                if (use_camera) {
                    cap.release();
                    cap.open(0);
                    continue;
                } else {
                    std::cout << "[INFO] Video ended" << std::endl;
                    break;
                }
            }
            
            frame_count++;
            
            // 检测装甲板
            auto armors = detector.detect(frame);
            auto debug_info = detector.getDebugInfo();
            auto lights = detector.getLights();
            cv::Mat binary = detector.getBinaryImage();
            
            // 更新跟踪器
            g_tracker.update(armors);
            
            cv::Mat display_frame = frame.clone();
            
            // 绘制灯条
            for (const auto& light : lights) {
                cv::Scalar light_color = (light.color == rm_auto_aim::RED) ? 
                    cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0);
                
                cv::Point2f vertices[4];
                light.rect.points(vertices);
                
                for (int j = 0; j < 4; j++) {
                    cv::line(display_frame, vertices[j], vertices[(j + 1) % 4], light_color, 1);
                }
                cv::circle(display_frame, light.center, 2, cv::Scalar(0, 255, 255), -1);
            }
            
            // 绘制装甲板
            for (const auto& armor : armors) {
                cv::Scalar armor_color = (armor.type == rm_auto_aim::ArmorType::SMALL) ? 
                    cv::Scalar(0, 255, 0) : cv::Scalar(0, 165, 255);
                armor.draw(display_frame, armor_color, 2);
                
                std::string coord_text = "(" + std::to_string((int)armor.center.x) + 
                                       ", " + std::to_string((int)armor.center.y) + ")";
                cv::putText(display_frame, coord_text, 
                           armor.center + cv::Point2f(5, -5),
                           cv::FONT_HERSHEY_SIMPLEX, 0.4, 
                           cv::Scalar(255, 255, 255), 1);
            }
            
            // 绘制跟踪器状态和预测位置
            if (g_tracker.isTracking()) {
                const auto* tracked_armor = g_tracker.getTrackedArmor();
                if (tracked_armor) {
                    // 绘制跟踪的装甲板（用不同颜色）
                    cv::Scalar tracking_color = cv::Scalar(255, 255, 0); // 黄色
                    
                    // 绘制装甲板边界
                    if (tracked_armor->vertices.size() == 4) {
                        for (size_t i = 0; i < 4; ++i) {
                            cv::line(display_frame, tracked_armor->vertices[i], 
                                    tracked_armor->vertices[(i + 1) % 4], 
                                    tracking_color, 3);
                        }
                    }
                    
                    // 绘制跟踪点
                    cv::circle(display_frame, tracked_armor->center, 5, 
                              cv::Scalar(0, 255, 255), -1);
                    
                    // 显示"TRACKED"文字
                    cv::putText(display_frame, "TRACKED", 
                               tracked_armor->center + cv::Point2f(-30, -30),
                               cv::FONT_HERSHEY_SIMPLEX, 0.7, tracking_color, 2);
                }
                
                // 绘制卡尔曼滤波预测位置
                cv::Point2f predicted_pos = g_tracker.getPredictedPosition();
                cv::circle(display_frame, predicted_pos, 8, 
                          cv::Scalar(255, 0, 255), -1); // 紫色圆点
                cv::circle(display_frame, predicted_pos, 8, 
                          cv::Scalar(255, 255, 255), 2); // 白色边框
                
                // 绘制预测箭头（速度方向）
                if (tracked_armor) {
                    cv::arrowedLine(display_frame, tracked_armor->center, 
                                   predicted_pos, cv::Scalar(255, 0, 255), 2);
                    
                    // 显示预测坐标
                    std::string pred_text = "Pred: (" + 
                                           std::to_string((int)predicted_pos.x) + 
                                           ", " + std::to_string((int)predicted_pos.y) + ")";
                    cv::putText(display_frame, pred_text, 
                               predicted_pos + cv::Point2f(10, 10),
                               cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                               cv::Scalar(255, 0, 255), 1);
                }
            }
            
            drawDebugInfo(display_frame, debug_info, g_params, g_tracker);
            
            std::string frame_text = "Frame: " + std::to_string(frame_count);
            cv::putText(display_frame, frame_text, 
                       cv::Point(frame.cols - 150, 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
            
            cv::imshow("Armor Detection + Tracking", display_frame);
            cv::imshow("Binary Mask", binary);
        }
        
        int key = cv::waitKey(paused ? 0 : 30);
        
        switch (key) {
            case 'q':
            case 27:
                std::cout << "[INFO] Quitting..." << std::endl;
                goto exit_loop;
                
            case 's':
            case 'S': {
                std::string filename = "frame_" + std::to_string(frame_count) + ".jpg";
                cv::imwrite(filename, frame);
                std::cout << "[INFO] Saved frame to: " << filename << std::endl;
                break;
            }
                
            case ' ':
                paused = !paused;
                std::cout << "[INFO] " << (paused ? "Paused" : "Resumed") << std::endl;
                break;
                
            case '+':
            case '=':
                g_params.hsv_red.v_min = std::min(255, g_params.hsv_red.v_min + 5);
                onTrackbarChange(0, nullptr);
                break;
                
            case '-':
            case '_':
                g_params.hsv_red.v_min = std::max(0, g_params.hsv_red.v_min - 5);
                onTrackbarChange(0, nullptr);
                break;
                
            case 'c':
            case 'C':
                g_params.detect_color = (g_params.detect_color == rm_auto_aim::RED) ? 
                                       rm_auto_aim::BLUE : rm_auto_aim::RED;
                onTrackbarChange(0, nullptr);
                std::cout << "[INFO] Detection color changed to: " 
                          << (g_params.detect_color == rm_auto_aim::RED ? "RED" : "BLUE") << std::endl;
                break;
                
            case 'r':
            case 'R':
                g_params.hsv_red.v_min = 100;
                g_params.hsv_red.s_min = 100;
                cv::setTrackbarPos("V_min", "Armor Detection + Tracking", g_params.hsv_red.v_min);
                cv::setTrackbarPos("S_min", "Armor Detection + Tracking", g_params.hsv_red.s_min);
                onTrackbarChange(0, nullptr);
                // 重置跟踪器
                g_tracker = rm_auto_aim::Tracker();
                std::cout << "[INFO] Parameters and tracker reset" << std::endl;
                break;
                
            case 't':
            case 'T':
                // 仅重置跟踪器
                g_tracker = rm_auto_aim::Tracker();
                std::cout << "[INFO] Tracker reset" << std::endl;
                break;
        }
    }
    
exit_loop:
    cap.release();
    cv::destroyAllWindows();
    
    std::cout << "\n[SUMMARY]" << std::endl;
    std::cout << "Total frames processed: " << frame_count << std::endl;
    std::cout << "Final parameters:" << std::endl;
    std::cout << "  Color: " << (g_params.detect_color == rm_auto_aim::RED ? "RED" : "BLUE") << std::endl;
    std::cout << "  V_min: " << g_params.hsv_red.v_min << std::endl;
    std::cout << "  S_min: " << g_params.hsv_red.s_min << std::endl;
    std::cout << "Tracker final state: ";
    switch (g_tracker.getState()) {
        case rm_auto_aim::Tracker::LOST: std::cout << "LOST"; break;
        case rm_auto_aim::Tracker::DETECTING: std::cout << "DETECTING"; break;
        case rm_auto_aim::Tracker::TRACKING: std::cout << "TRACKING"; break;
        case rm_auto_aim::Tracker::TEMP_LOST: std::cout << "TEMP_LOST"; break;
    }
    std::cout << std::endl;
    std::cout << "\n✅ Task 2.2.1.3 - Kalman Filter Tracking COMPLETED!" << std::endl;
    
    return 0;
}