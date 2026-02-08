#pragma once

namespace auto_aim_interfaces {
namespace msg {

struct DebugLight {
    float center_x = 0.0f;
    float center_y = 0.0f;
    float width = 0.0f;
    float length = 0.0f;
    float ratio = 0.0f;
    float angle = 0.0f;
    bool is_light = false;
    int color = 0;  // 0=红色, 1=蓝色
};

} // namespace msg
} // namespace auto_aim_interfaces