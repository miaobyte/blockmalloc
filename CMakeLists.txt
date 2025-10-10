cmake_minimum_required(VERSION 3.10)
project(blockmalloc VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(GNUInstallDirs)

# 共享库
add_library(blockmalloc SHARED
    src/blockmalloc.c
    src/spinlock.c
)

set_target_properties(blockmalloc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
)

target_include_directories(blockmalloc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(blockmalloc PRIVATE ENABLE_LOG)
endif()

# 安装库与头文件
install(TARGETS blockmalloc
    EXPORT blockmallocTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets and generate CMake config package and pkg-config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/blockmallocConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/BlockmallocConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/blockmallocConfig.cmake"
    @ONLY
)

install(EXPORT blockmallocTargets
    FILE blockmallocTargets.cmake
    NAMESPACE blockmalloc::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/blockmalloc-${PROJECT_VERSION}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blockmallocConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/blockmallocConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/blockmalloc-${PROJECT_VERSION})

# 测试
enable_testing()
add_subdirectory(test)