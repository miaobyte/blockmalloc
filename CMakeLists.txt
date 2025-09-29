cmake_minimum_required(VERSION 3.10)
project(block_malloc VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(GNUInstallDirs)

# 共享库
add_library(block_malloc SHARED
    src/block_malloc.c
    src/spinlock.c
)

# 版本 / SONAME，解决 ${shlibs:Depends} 未生成问题
set_target_properties(block_malloc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
)

target_include_directories(block_malloc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(block_malloc PRIVATE ENABLE_LOG)
endif()

# 安装库与头文件
install(TARGETS block_malloc
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 测试
enable_testing()
add_subdirectory(test)