# RoboMaster 视觉识别系统

GitHub 项目地址：https://github.com/Chen-yx1/rm-vision-newtest

## 项目背景
本项目基于 [chenjunnn/rm_auto_aim](https://github.com/chenjunnn/rm_auto_aim?tab=readme-ov-file) 进行了重构.

## 核心改进
- **算法优化**：改进了灯条匹配和装甲板识别算法
- **代码重构**：将原项目模块化，提高可读性和可维护性
- **工具链完善**：增加了相机标定、测试视频生成等实用工具
- **服务集成**：添加了Systemd服务支持，便于实际部署

## 快速运行指南

### 1. 克隆项目
```bash
git clone https://github.com/Chen-yx1/rm-vision-newtest.git
cd rm-vision-newtest
```

### 2. 安装依赖（Ubuntu）
```bash
sudo apt update
sudo apt install -y build-essential cmake libopencv-dev
```

### 3. 一键编译
```bash
./run.sh
```

### 4. 运行程序
```bash
# 使用摄像头（确保摄像头已连接）
./bin/rm_vision_newtest camera

# 或使用测试视频（需先放置视频文件）
./bin/rm_vision_newtest test_video.mp4
```

## 主要功能演示
1. **装甲板识别**：实时检测红/蓝方装甲板
2. **3D定位**：输出目标距离、角度信息
3. **目标跟踪**：使用卡尔曼滤波器平滑跟踪

## 关键文件位置
- 核心识别算法：`src/detector.cpp`
- 3D坐标解算：`src/pnp_solver.cpp`
- 项目主入口：`src/main.cpp`

## 鸣谢
特别感谢 [chenjunnn/rm_auto_aim](https://github.com/chenjunnn/rm_auto_aim) 项目提供的算法基础和代码框架，为我的项目开发奠定了重要基础。

##环境
Ubuntu 22.04

OpenCV 4.5

CMake 3.10
