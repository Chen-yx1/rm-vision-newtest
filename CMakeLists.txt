cmake_minimum_required(VERSION 3.10)
project(rm_vision_newtest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4.5 REQUIRED COMPONENTS core imgproc highgui videoio calib3d video)

if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# 查找Eigen3库
find_package(Eigen3 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS} ${Eigen3_INCLUDE_DIRS})

set(SOURCE_FILES
    src/armor.cpp
    src/detector.cpp
    src/pnp_solver.cpp
    src/kalman_filter.cpp    # 新增
    src/tracker.cpp          # 新增
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} Eigen3::Eigen)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

message(STATUS "Build configuration complete")