cmake_minimum_required(VERSION 3.8)

project(main)

# 设置交叉编译工具链
set(TOOLCHAIN_DIR "/usr")
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++")

# 静态链接配置
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)

# 确保优先查找静态库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 包含头文件目录
include_directories(.)

# 添加LVGL子项目
add_subdirectory(lvgl)
add_subdirectory(lv_drivers)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

# 添加可执行文件
add_executable(${PROJECT_NAME} main.c mouse_cursor_icon.c )

# 链接LVGL库和必要的系统库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    lvgl 
    lvgl::examples 
    lvgl::demos 
    lvgl::drivers
    pthread
    m
    rt
    dl
)

# 为ARM架构添加特定编译选项
target_compile_options(${PROJECT_NAME} PRIVATE
    -march=armv7-a  # 根据您的开发板架构调整
    -mtune=cortex-a9 # 根据您的CPU型号调整
    -O2
    -Wall
)
