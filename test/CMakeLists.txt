
add_executable(block_test
    blocks.c
)

target_link_libraries(block_test block_malloc)
