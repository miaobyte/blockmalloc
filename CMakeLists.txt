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
    EXPORT block_mallocTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets and generate CMake config package and pkg-config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/block_mallocConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/BlockMallocConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/block_mallocConfig.cmake"
    @ONLY
)

install(EXPORT block_mallocTargets
    FILE block_mallocTargets.cmake
    NAMESPACE block_malloc::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/block_malloc-${PROJECT_VERSION}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/block_mallocConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/block_mallocConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/block_malloc-${PROJECT_VERSION})

# 测试
enable_testing()
add_subdirectory(test)