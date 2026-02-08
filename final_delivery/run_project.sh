#!/bin/bash
echo "=== RoboMaster Vision Project Runner ==="
echo "This script compiles and runs the project with minimal memory usage"

# 创建build目录
mkdir -p build
cd build

# 清理旧文件
rm -rf *

# 配置CMake
echo "Step 1: Configuring project..."
cmake .. > cmake.log 2>&1

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed"
    echo "See cmake.log for details"
    exit 1
fi

echo "✅ Configuration successful"

# 逐个编译以节省内存
echo "Step 2: Compiling project (one file at a time)..."
echo "This may take a few minutes..."

# 编译每个源文件
for file in ../src/*.cpp; do
    filename=$(basename "$file" .cpp)
    echo "  Compiling $filename.cpp..."
    make rm_vision_final.dir/src/$filename.cpp.o -j1 > compile_${filename}.log 2>&1
    
    if [ $? -ne 0 ]; then
        echo "❌ Failed to compile $filename.cpp"
        echo "See compile_${filename}.log for details"
        exit 1
    fi
    
    echo "  ✓ $filename.cpp done"
done

# 链接
echo "Step 3: Linking..."
make rm_vision_final -j1 > link.log 2>&1

if [ $? -ne 0 ]; then
    echo "❌ Linking failed"
    echo "See link.log for details"
    exit 1
fi

echo "✅ Compilation successful!"
cd ..

# 创建测试图片
echo "Step 4: Creating test image..."
python3 -c "
import cv2, numpy as np
img = np.zeros((480,640,3), np.uint8)
cv2.rectangle(img, (280,190), (300,290), (0,0,255), -1)
cv2.rectangle(img, (340,190), (360,290), (0,0,255), -1)
cv2.imwrite('test_image.jpg', img)
print('Test image created: test_image.jpg')
"

# 运行程序
echo "Step 5: Running program..."
echo "========================================"
./build/rm_vision_final test_image.jpg
echo "========================================"

echo ""
echo "✅ Project execution complete!"
echo "If you see the tasks marked as COMPLETE, all 4 tasks are successfully implemented."
