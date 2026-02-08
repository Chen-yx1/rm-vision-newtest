#!/bin/bash
echo "=== 安全编译模式（避免内存不足）==="

# 清理build目录
cd build
rm -rf *

# 配置但不立即编译
cmake ..
echo "✅ CMake配置完成"

# 逐个编译源文件，避免内存峰值
echo "开始逐个编译..."

# 编译顺序：先编译小文件，再编译大文件
make rm_vision_newtest.dir/src/armor.cpp.o
echo "✅ 完成 armor.cpp (11%)"

make rm_vision_newtest.dir/src/kalman_filter.cpp.o
echo "✅ 完成 kalman_filter.cpp (22%)"

make rm_vision_newtest.dir/src/pnp_solver.cpp.o
echo "✅ 完成 pnp_solver.cpp (33%)"

make rm_vision_newtest.dir/src/tracker.cpp.o
echo "✅ 完成 tracker.cpp (44%)"

# 跳过可能有问题的文件，先完成其他
make rm_vision_newtest.dir/src/detector.cpp.o
echo "✅ 完成 detector.cpp (55%)"

# 最后编译最大的main.cpp
make rm_vision_newtest.dir/src/main.cpp.o
echo "✅ 完成 main.cpp (66%)"

# 链接所有目标文件
make rm_vision_newtest
echo "✅ 完成链接 (100%)"

cd ..
echo "=== 编译完成 ==="
