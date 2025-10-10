add_executable(block_simpletest 1_block_simpletest.c)
target_link_libraries(block_simpletest PRIVATE blockmalloc)

add_executable(blockmallocmax 2_block_mallocmax.c)
target_link_libraries(blockmallocmax PRIVATE blockmalloc)

add_test(NAME block_simpletest COMMAND block_simpletest)
add_test(NAME blockmallocmax COMMAND blockmallocmax)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(block_simpletest PRIVATE ENABLE_LOG)
    target_compile_definitions(blockmallocmax PRIVATE ENABLE_LOG)
endif()