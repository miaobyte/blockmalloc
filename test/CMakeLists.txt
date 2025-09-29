add_executable(block_simpletest 1_block_simpletest.c)
target_link_libraries(block_simpletest PRIVATE block_malloc)

add_executable(block_mallocmax 2_block_mallocmax.c)
target_link_libraries(block_mallocmax PRIVATE block_malloc)

add_test(NAME block_simpletest COMMAND block_simpletest)
add_test(NAME block_mallocmax COMMAND block_mallocmax)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(block_simpletest PRIVATE ENABLE_LOG)
    target_compile_definitions(block_mallocmax PRIVATE ENABLE_LOG)
endif()