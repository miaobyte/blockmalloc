#include <assert.h>

#include "block_malloc.h"

void test_alloc(blocks_meta_t *blocks, void *block_start) {
    for (size_t i = 0; i < 50; i++) {
        int64_t block_id= blocks_alloc(blocks, block_start);
        assert(block_id != (size_t)-1);
    }
}

void main() {
    uint8_t pool[1024 * 4];
    blocks_meta_t blocks;
    blocks_init(&blocks,1024 * 4, 64);
    test_alloc(&blocks, pool);
}