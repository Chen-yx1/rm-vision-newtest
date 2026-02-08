#!/bin/bash
# RoboMaster视觉识别系统 - 一键启动脚本
# 版本: 2.2.1.5

echo "=========================================="
echo "RoboMaster视觉识别系统启动中..."
echo "=========================================="

# 进入项目目录
cd "$(dirname "$0")"

# 检查依赖
echo "[1/5] 检查依赖..."
if ! pkg-config --exists opencv4; then
    echo "⚠️ 未找到OpenCV，正在安装..."
    sudo apt-get update
    sudo apt-get install -y libopencv-dev
fi

if ! command -v python3 &> /dev/null; then
    echo "⚠️ 未找到Python3，正在安装..."
    sudo apt-get install -y python3 python3-pip
fi

# 编译项目
echo "[2/5] 编译项目..."
if [ ! -d "build" ]; then
    mkdir -p build
fi

cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# 检查可执行文件
echo "[3/5] 检查可执行文件..."
if [ -f "bin/rm_vision_newtest" ]; then
    EXEC_PATH="./bin/rm_vision_newtest"
elif [ -f "build/rm_vision_newtest" ]; then
    EXEC_PATH="./build/rm_vision_newtest"
else
    echo "❌ 错误：找不到可执行文件"
    exit 1
fi

# 运行程序
echo "[4/5] 启动视觉识别程序..."
echo "=========================================="
echo "程序已启动！"
echo "控制台输出："
echo "=========================================="

# 尝试不同的视频源
VIDEO_SOURCES=("test_video.mp4" "test1_video.mp4" "camera")

for source in "${VIDEO_SOURCES[@]}"; do
    if [[ "$source" == "camera" ]] || [ -f "$source" ]; then
        echo "尝试使用视频源: $source"
        $EXEC_PATH "$source"
        break
    fi
done

echo "=========================================="
echo "程序已退出"
echo "=========================================="
