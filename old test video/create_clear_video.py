#!/usr/bin/env python3
"""
创建非常明显的装甲板测试视频
确保程序一定能检测到
"""

import cv2
import numpy as np

print("🎬 创建明显测试视频...")

# 视频参数
width, height = 640, 480
fps = 30
duration = 6
total_frames = fps * duration

# 创建视频
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
video_path = "test_clear_video.mp4"
out = cv2.VideoWriter(video_path, fourcc, fps, (width, height))

for i in range(total_frames):
    # 创建黑色背景
    frame = np.zeros((height, width, 3), dtype=np.uint8)
    
    # 前3秒：非常亮的红色装甲板
    if i < fps * 3:
        # 两个非常亮的红色灯条（完全饱和）
        cv2.rectangle(frame, (250, 200), (260, 280), (0, 0, 255), -1)  # 纯红色
        cv2.rectangle(frame, (290, 200), (300, 280), (0, 0, 255), -1)  # 纯红色
        
        # 装甲板轮廓（绿色）
        cv2.rectangle(frame, (255, 210), (295, 270), (0, 255, 0), 2)
        
        # 文字
        cv2.putText(frame, "BRIGHT RED ARMOR", (220, 190), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
    
    # 后3秒：非常亮的蓝色装甲板
    else:
        # 两个非常亮的蓝色灯条
        cv2.rectangle(frame, (350, 200), (360, 280), (255, 0, 0), -1)  # 纯蓝色
        cv2.rectangle(frame, (390, 200), (400, 280), (255, 0, 0), -1)  # 纯蓝色
        
        # 装甲板轮廓
        cv2.rectangle(frame, (355, 210), (395, 270), (0, 255, 0), 2)
        
        # 文字
        cv2.putText(frame, "BRIGHT BLUE ARMOR", (320, 190), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 0), 2)
    
    # 添加帧信息
    cv2.putText(frame, f"Frame: {i}", (10, 20), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
    cv2.putText(frame, f"Time: {i/fps:.1f}s", (10, 40), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
    
    # 添加提示
    cv2.putText(frame, "V_min: 30-50, S_min: 30-50", (10, 60), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 0), 1)
    
    out.write(frame)
    
    # 进度
    if i % 30 == 0:
        print(f"  进度: {i}/{total_frames}")

out.release()
print(f"✅ 明显测试视频已创建: {video_path}")
print("🎯 视频内容:")
print("   0-3秒: 非常亮的红色装甲板")
print("   3-6秒: 非常亮的蓝色装甲板")
print("📌 运行: ./bin/rm_vision_newtest test_clear_video.mp4")
