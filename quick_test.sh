#!/bin/bash

echo "=== 快速测试脚本 ==="
echo

# 创建测试图片
echo "1. 创建测试图片..."
python3 -c "
import cv2
import numpy as np

# 创建简单测试图片
img = np.zeros((480, 640, 3), dtype=np.uint8)

# 左灯条（竖直）
cv2.rectangle(img, (280, 190), (300, 290), (0, 0, 255), -1)

# 右灯条（竖直）
cv2.rectangle(img, (340, 190), (360, 290), (0, 0, 255), -1)

# 保存
cv2.imwrite('quick_test.jpg', img)
print('  创建: quick_test.jpg')
"

echo
echo "2. 编译项目..."
cd build
make clean
make -j2
cd ..

echo
echo "3. 测试检测器..."
if [ -f "bin/rm_vision_newtest" ]; then
    ./bin/rm_vision_newtest quick_test.jpg
elif [ -f "build/rm_vision_newtest" ]; then
    ./build/rm_vision_newtest quick_test.jpg
else
    echo "  错误：找不到可执行文件"
fi

echo
echo "=== 测试完成 ==="
