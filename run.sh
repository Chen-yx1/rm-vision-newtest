#!/bin/bash
echo "==========================================="
echo "RoboMaster 视觉考核项目 - 装甲板识别"
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

echo "[2/5] 检查视频文件..."
VIDEO_FILE="test_video.mp4"
CAMERA_MODE=""

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
echo "启动装甲板检测程序..."
echo "按 'q' 退出，按空格暂停"
echo ""

# 检查可执行文件在哪里
if [ -f "bin/rm_vision_newtest" ]; then
    echo "✅ 在 bin/ 目录找到可执行文件"
    if [ -z "$CAMERA_MODE" ]; then
        ./bin/rm_vision_newtest "$VIDEO_FILE"
    else
        ./bin/rm_vision_newtest camera
    fi
elif [ -f "build/rm_vision_newtest" ]; then
    echo "✅ 在 build/ 目录找到可执行文件"
    if [ -z "$CAMERA_MODE" ]; then
        ./build/rm_vision_newtest "$VIDEO_FILE"
    else
        ./build/rm_vision_newtest camera
    fi
else
    echo "❌ 未找到可执行文件"
    echo "请在以下目录中查找可执行文件:"
    ls -la bin/ 2>/dev/null || echo "bin/ 目录不存在"
    ls -la build/ 2>/dev/null || echo "build/ 目录不存在"
    exit 1
fi
