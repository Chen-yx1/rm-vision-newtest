#!/bin/bash
echo "=== 快速测试视觉系统 ==="

# 创建测试图片
echo "1. 创建测试图片..."
python3 -c "
import cv2, numpy as np
img = np.zeros((480,640,3), np.uint8)
cv2.rectangle(img, (280,190), (300,290), (0,0,255), -1)
cv2.rectangle(img, (340,190), (360,290), (0,0,255), -1)
cv2.imwrite('test_quick.jpg', img)
print('  创建: test_quick.jpg')
"

# 检查程序是否存在
echo "2. 检查程序..."
if [ -f "bin/rm_vision_newtest" ]; then
    echo "✅ 找到可执行文件: bin/rm_vision_newtest"
    ./bin/rm_vision_newtest test_quick.jpg
elif [ -f "build/rm_vision_newtest" ]; then
    echo "✅ 找到可执行文件: build/rm_vision_newtest"
    ./build/rm_vision_newtest test_quick.jpg
else
    echo "❌ 错误：找不到可执行文件"
    echo "请先运行: cd build && make && cd .."
fi
