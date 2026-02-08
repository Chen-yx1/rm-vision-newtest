#!/bin/bash
# 快速启动脚本 - 直接运行已编译的程序
echo "=========================================="
echo "快速启动 RoboMaster 视觉系统"
echo "启动时间: $(date)"
echo "=========================================="

cd "$(dirname "$0")"

# 直接运行程序，不编译
if [ -f "bin/rm_vision_newtest" ]; then
    echo "✅ 使用 bin/rm_vision_newtest"
    ./bin/rm_vision_newtest camera
elif [ -f "build/rm_vision_newtest" ]; then
    echo "✅ 使用 build/rm_vision_newtest"
    ./build/rm_vision_newtest camera
else
    echo "❌ 错误：找不到可执行文件"
    echo "请先手动编译：cd build && make && cd .."
    exit 1
fi
