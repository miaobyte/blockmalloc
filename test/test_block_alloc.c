#include <stdio.h>
#include <assert.h>

#include "block_malloc.h"

void test_alloc(blocks_meta_t *blocks) {

    for (size_t i = 0; i < 62; i++) {
        int64_t block_id= blocks_alloc(blocks);
        assert(block_id != (size_t)-1);
    }
}

void main() {
    uint8_t pool[1024 * 4];
    blocks_meta_t blocks;
    blocks_init(pool,1024 * 4, 64, &blocks);
    test_alloc(&blocks);
}