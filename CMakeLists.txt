cmake_minimum_required(VERSION 3.10)
project(block_malloc)

set(CMAKE_C_STANDARD 11)

# 添加头文件路径
include_directories(include)
include_directories(src)

# 添加源文件
add_library(block_malloc SHARED
    src/block_malloc.c
)

add_subdirectory(test)

# 安装目标和头文件
install(TARGETS block_malloc DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)