RoboMaster 装甲板识别系统
##已完成功能
2.2.1.1：灯条识别（红/蓝颜色提取、轮廓检测）

2.2.1.2：装甲板匹配（左右灯条配对、中心点计算）

2.2.1.3：卡尔曼滤波（目标跟踪、位置预测）

##快速开始
bash
chmod +x run.sh
./run.sh                      # 使用视频文件
./run.sh camera              # 使用摄像头
##交互控制
按键	功能
q/ESC	退出程序
空格	暂停/继续
s	保存当前帧
+/-	调整V_min阈值
c	切换红蓝颜色
r	重置所有参数
##实时调节
V_min滑块：调整亮度阈值（0-255）

S_min滑块：调整饱和度阈值（0-255）

##项目结构
text
rm-vision-newtest/
├── include/          # 头文件
├── src/             # 源文件
├── CMakeLists.txt   # 构建配置
├── run.sh           # 一键运行脚本
└── test_video.mp4   # 测试视频
##编译运行
bash
mkdir build && cd build
cmake .. && make
cd ..
./bin/rm_vision_newtest test_video.mp4


##环境
Ubuntu 22.04

OpenCV 4.5

CMake 3.10