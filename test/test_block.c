#include <stdio.h>
#include <assert.h>

#include "block_malloc.h"

#include <stdlib.h> // 用于随机数生成
#include <time.h>   // 用于随机数种子
void test_free(blocks_meta_t *blocks) {
    int64_t block_id = blocks_alloc(blocks);
    for(int i=0;i<10;i++){
         block_id = blocks_alloc(blocks);
    }
    blocks_free(blocks,9);
    blocks_alloc(blocks);
    blocks_free(blocks,9);
    blocks_free(blocks,3);
    blocks_alloc(blocks);
    blocks_free(blocks,5);
    blocks_free(blocks,4);
    blocks_alloc(blocks);
}

int main(int argc, char *argv[]) {
 
    uint8_t pool[1024 * 4];
    blocks_meta_t blocks;
    blocks_init(pool,1024 * 4, 64, &blocks);
    printf("blocks inited\n");
    test_free(&blocks);
    return 0;
}