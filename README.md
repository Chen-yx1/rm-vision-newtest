已完成
2.2.1.1：灯条识别（红蓝二值化、轮廓提取、矩形框选）

2.2.1.2：装甲板匹配（左右灯条配对、中心点拟合、误识别剔除）

编译运行
bash
./run.sh                    # 一键构建运行
# 或
cd build && cmake .. && make
./rm_vision_newtest 视频文件  # 运行检测
功能特性
红蓝双色识别

实时调试界面

参数动态调整

装甲板分类（大小）




使用方法
./rm_vision_newtest test_video.mp4 - 使用视频

./rm_vision_newtest camera - 使用摄像头

控制按键：

q - 退出

s - 保存截图

空格 - 暂停/继续

+/- - 调整阈值

功能
识别红色/蓝色灯条

实时参数调整

显示二值化图像

环境
Ubuntu 22.04

OpenCV 4.5

CMake 3.10