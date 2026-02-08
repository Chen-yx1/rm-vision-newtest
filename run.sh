#!/bin/bash

echo "==========================================="
echo "RoboMaster 视觉考核项目 - 装甲板识别"
echo "版本: 2.2.1.4 (3D坐标转换)"
echo "==========================================="

if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ 错误：请在项目根目录运行此脚本！"
    exit 1
fi

echo "[1/5] 检查依赖..."

# 检查OpenCV
if ! pkg-config --exists opencv4; then
    echo "⚠️  未找到OpenCV，尝试安装..."
    sudo apt-get update
    sudo apt-get install -y libopencv-dev
fi

# 检查Python和OpenCV for Python（用于标定脚本）
if ! command -v python3 &> /dev/null; then
    echo "⚠️  未找到Python3，尝试安装..."
    sudo apt-get install -y python3 python3-pip
fi

# 检查Python OpenCV
if ! python3 -c "import cv2" 2>/dev/null; then
    echo "⚠️  未找到Python OpenCV，尝试安装..."
    pip3 install opencv-python numpy
fi

echo "[2/5] 检查视频文件和标定数据..."

VIDEO_FILE="test_video.mp4"
CALIB_FILE="camera_calibration.yml"
CALIB_DIR="./calibration_images"
CAMERA_MODE=""

# 检查视频文件
if [ ! -f "$VIDEO_FILE" ]; then
    echo "⚠️  未找到视频文件 '$VIDEO_FILE'"
    read -p "是否使用摄像头？(y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        CAMERA_MODE="camera"
        echo "✅ 将使用摄像头"
    else
        echo "❌ 请将 test_video.mp4 放置在项目根目录"
        exit 1
    fi
else
    echo "✅ 找到视频文件: $VIDEO_FILE"
fi

# 检查标定文件
if [ -f "$CALIB_FILE" ]; then
    echo "✅ 找到相机标定文件: $CALIB_FILE"
    echo "   程序将自动加载标定参数"
else
    echo "⚠️  未找到相机标定文件"
    echo "   可以使用虚拟参数运行，或执行相机标定"
fi

# 检查标定图像目录
if [ ! -d "$CALIB_DIR" ]; then
    echo "⚠️  未找到标定图像目录 $CALIB_DIR"
    echo "   如果需要执行相机标定，请将棋盘格图像放入此目录"
    mkdir -p "$CALIB_DIR" 2>/dev/null
    echo "   已创建目录: $CALIB_DIR"
    echo "   你可以运行: python calibrate_camera.py generate 生成测试图像"
else
    IMAGE_COUNT=$(ls "$CALIB_DIR"/*.jpg "$CALIB_DIR"/*.png 2>/dev/null | wc -l)
    if [ "$IMAGE_COUNT" -gt 0 ]; then
        echo "✅ 标定图像目录: $CALIB_DIR (包含 $IMAGE_COUNT 张图像)"
    else
        echo "⚠️  标定图像目录为空"
        echo "   请放置棋盘格图像或运行: python calibrate_camera.py generate"
    fi
fi

echo "[3/5] 清理旧构建..."
rm -rf build/*
mkdir -p bin build

echo "[4/5] 配置项目..."
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "❌ CMake配置失败！"
    exit 1
fi

echo "[5/5] 编译项目..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ 编译失败！"
    exit 1
fi

cd ..
echo "✅ 编译完成！"

echo ""
echo "==========================================="
echo "启动装甲板检测程序..."
echo "==========================================="
echo ""
echo "🎮 控制说明:"
echo "  基本控制:"
echo "    q / ESC - 退出程序"
echo "    空格     - 暂停/继续"
echo "    s       - 保存当前帧"
echo "    +/-     - 调整V_min阈值"
echo "    c       - 切换红蓝颜色"
echo "    r       - 重置所有参数"
echo ""
echo "  3D坐标控制:"
echo "    3       - 切换3D坐标显示"
echo "    l       - 加载标定参数"
echo "    d       - 使用虚拟参数"
echo "    C       - 执行相机标定(提示)"
echo ""
echo "  标定工具:"
echo "    python calibrate_camera.py generate    # 生成测试图像"
echo "    python calibrate_camera.py calibrate   # 执行相机标定"
echo ""
echo "==========================================="
echo ""

# 检查可执行文件在哪里
if [ -f "bin/rm_vision_newtest" ]; then
    echo "✅ 在 bin/ 目录找到可执行文件"
    EXEC_PATH="./bin/rm_vision_newtest"
elif [ -f "build/rm_vision_newtest" ]; then
    echo "✅ 在 build/ 目录找到可执行文件"
    EXEC_PATH="./build/rm_vision_newtest"
else
    echo "❌ 未找到可执行文件"
    echo "请在以下目录中查找可执行文件:"
    ls -la bin/ 2>/dev/null || echo "bin/ 目录不存在"
    ls -la build/ 2>/dev/null || echo "build/ 目录不存在"
    exit 1
fi

# 运行程序
if [ -z "$CAMERA_MODE" ]; then
    $EXEC_PATH "$VIDEO_FILE"
else
    $EXEC_PATH camera
fi

echo ""
echo "程序已退出"
echo "标定数据保存在: $CALIB_FILE"
echo "标定图像目录: $CALIB_DIR"
echo ""
echo "下次运行可以直接使用: $EXEC_PATH [video_file|camera]"