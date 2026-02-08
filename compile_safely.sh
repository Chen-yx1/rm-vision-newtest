#!/bin/bash
echo "=== 安全编译脚本 ==="

# 清理build目录
cd build
rm -rf *

# 配置CMake但不编译
echo "配置CMake..."
cmake .. > cmake_log.txt 2>&1

if [ $? -ne 0 ]; then
    echo "❌ CMake配置失败"
    cat cmake_log.txt
    exit 1
fi

echo "✅ CMake配置成功"

# 逐个编译，避免内存峰值
echo "开始逐个编译..."

# 使用make的单个目标编译，每次只编译一个文件
for target in armor detector pnp_solver kalman_filter tracker camera_calibrator coordinate_transformer main; do
    echo "编译 $target.cpp..."
    make rm_vision_newtest.dir/src/$target.cpp.o -j1 2>&1 | tee -a compile_log.txt
    
    if [ $? -ne 0 ]; then
        echo "❌ $target.cpp 编译失败"
        exit 1
    fi
    
    echo "✅ $target.cpp 完成"
    sleep 1  # 给系统一些时间释放内存
done

# 链接
echo "链接所有目标文件..."
make rm_vision_newtest -j1 2>&1 | tee -a compile_log.txt

if [ $? -eq 0 ]; then
    echo "✅ 编译成功！"
    cd ..
    echo "可执行文件：build/rm_vision_newtest"
else
    echo "❌ 链接失败"
    exit 1
fi
