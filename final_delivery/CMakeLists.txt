cmake_minimum_required(VERSION 3.16)
project(rm_vision_final)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置优化级别为O1以减少内存使用
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

find_package(OpenCV REQUIRED)

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})

# 源文件列表
set(SOURCE_FILES
    src/armor.cpp
    src/detector.cpp
    src/kalman_filter.cpp
    src/tracker.cpp
    src/pnp_solver.cpp
    src/camera_calibrator.cpp
    src/coordinate_transformer.cpp
    src/main.cpp
)

add_executable(rm_vision_final ${SOURCE_FILES})
target_link_libraries(rm_vision_final ${OpenCV_LIBS})

# 编译后提示
message(STATUS "RoboMaster Vision Project configured successfully")
message(STATUS "To compile with minimal memory: make -j1")
message(STATUS "Executable will be: rm_vision_final")
