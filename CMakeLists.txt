cmake_minimum_required(VERSION 3.10)
project(rm_vision_newtest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCV
find_package(OpenCV 4.5 REQUIRED COMPONENTS core imgproc highgui videoio calib3d)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# 源文件
set(SOURCE_FILES
    src/armor.cpp
    src/detector.cpp
    src/pnp_solver.cpp
    src/kalman_filter.cpp
    src/tracker.cpp
    src/camera_calibrator.cpp
    src/coordinate_transformer.cpp
    src/main.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# 链接库
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

message(STATUS "Build configuration complete")