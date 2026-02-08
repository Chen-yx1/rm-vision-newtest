#!/usr/bin/env python3
"""
简单测试视频生成脚本
生成包含红蓝装甲板灯条的测试视频
"""

import numpy as np

# 模拟opencv的cv2模块，如果你没有安装opencv-python
try:
    import cv2
    OPENCV_AVAILABLE = True
except ImportError:
    OPENCV_AVAILABLE = False
    print("警告: opencv-python未安装，将创建替代方案")
    
def create_simple_video():
    print("🎬 开始生成简单测试视频...")
    
    # 视频参数
    width, height = 640, 480
    fps = 30
    duration = 5  # 5秒
    total_frames = fps * duration
    
    if OPENCV_AVAILABLE:
        # 使用OpenCV创建视频
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter('test_video.mp4', fourcc, fps, (width, height))
        
        for i in range(total_frames):
            # 创建黑色背景
            frame = np.zeros((height, width, 3), dtype=np.uint8)
            
            # 添加一些随机噪声
            noise = np.random.randint(0, 30, (height, width, 3), dtype=np.uint8)
            frame = cv2.add(frame, noise)
            
            # 前2.5秒：红色装甲板
            if i < 75:  # 2.5秒 * 30fps
                # 两个红色矩形作为灯条
                cv2.rectangle(frame, (250, 200), (260, 280), (0, 0, 255), -1)  # 左灯条
                cv2.rectangle(frame, (290, 200), (300, 280), (0, 0, 255), -1)  # 右灯条
                
                # 装甲板轮廓
                cv2.rectangle(frame, (255, 210), (295, 270), (0, 255, 0), 2)
                
                # 添加文字
                cv2.putText(frame, "RED ARMOR", (240, 190), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
            
            # 后2.5秒：蓝色装甲板
            else:
                # 两个蓝色矩形作为灯条
                cv2.rectangle(frame, (350, 200), (360, 280), (255, 0, 0), -1)  # 左灯条
                cv2.rectangle(frame, (390, 200), (400, 280), (255, 0, 0), -1)  # 右灯条
                
                # 装甲板轮廓
                cv2.rectangle(frame, (355, 210), (395, 270), (0, 255, 0), 2)
                
                # 添加文字
                cv2.putText(frame, "BLUE ARMOR", (340, 190), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)
            
            # 添加帧编号
            cv2.putText(frame, f"Frame: {i}", (10, 20), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            cv2.putText(frame, f"Time: {i/fps:.1f}s", (10, 40), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            
            out.write(frame)
            
            # 显示进度
            if i % 30 == 0:
                print(f"  进度: {i}/{total_frames} 帧")
        
        out.release()
        print(f"✅ 视频生成完成: test_video.mp4")
        
    else:
        # 如果没有OpenCV，创建一系列PNG图片
        print("创建PNG图片序列代替视频...")
        
        for i in range(min(total_frames, 30)):  # 只创建30张图片作为测试
            # 创建黑色背景
            frame = np.zeros((height, width, 3), dtype=np.uint8)
            
            # 添加一些随机噪声
            noise = np.random.randint(0, 30, (height, width, 3), dtype=np.uint8)
            frame = frame + noise
            frame = np.clip(frame, 0, 255).astype(np.uint8)
            
            # 红色装甲板
            # 两个红色矩形作为灯条
            frame[200:280, 250:260] = [0, 0, 255]  # 左灯条
            frame[200:280, 290:300] = [0, 0, 255]  # 右灯条
            
            # 装甲板轮廓（绿色边框）
            for x in range(255, 296):
                frame[210, x] = [0, 255, 0]
                frame[270, x] = [0, 255, 0]
            for y in range(210, 271):
                frame[y, 255] = [0, 255, 0]
                frame[y, 295] = [0, 255, 0]
            
            # 保存为PNG
            from PIL import Image
            img = Image.fromarray(frame)
            img.save(f"test_frame_{i:03d}.png")
            
            if i % 5 == 0:
                print(f"  生成图片: test_frame_{i:03d}.png")
        
        print("✅ PNG图片序列生成完成")
        print("   使用这些图片进行测试，或安装opencv-python生成视频")
        print("   安装命令: pip3 install opencv-python")

if __name__ == "__main__":
    create_simple_video()
