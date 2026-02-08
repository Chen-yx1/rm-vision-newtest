import cv2
import numpy as np

# 创建纯红色灯条图片
img = np.zeros((480, 640, 3), dtype=np.uint8)

# 画两个纯红色矩形（BGR格式：0,0,255）
img[200:280, 250:260] = [0, 0, 255]  # 左灯条
img[200:280, 290:300] = [0, 0, 255]  # 右灯条

cv2.imwrite('test_minimal.jpg', img)
print("创建了最简单的测试图片: test_minimal.jpg")
print("这两个红色矩形的RGB值都是 (255, 0, 0)，应该很容易检测")
