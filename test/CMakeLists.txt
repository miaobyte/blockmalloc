add_executable(test_block_alloc test_block_alloc.c)
target_link_libraries(test_block_alloc block_malloc)


add_executable(test_block test_block.c)
target_link_libraries(test_block block_malloc)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(test_block_alloc PRIVATE ENABLE_LOG)
    target_compile_definitions(test_block PRIVATE ENABLE_LOG)
endif()