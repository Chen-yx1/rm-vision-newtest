cmake_minimum_required(VERSION 3.10)
project(rm-vision-newtest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(OpenCV 4.5 REQUIRED COMPONENTS core imgproc highgui videoio)

message(STATUS "项目: ${PROJECT_NAME}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenCV 版本: ${OpenCV_VERSION}")

include_directories(${CMAKE_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/detector.cpp
)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_DEBUG_OUTPUT)
    message(STATUS "启用调试模式")
endif()

message(STATUS "✅ CMake 配置完成")
message(STATUS "运行: make 进行编译")
message(STATUS "运行: ./${PROJECT_NAME} test_video.mp4 启动程序")