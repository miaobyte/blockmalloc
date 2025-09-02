#include <stdio.h>
#include <assert.h>

#include "block_malloc.h"
#include "logutil.h"

void test_alloc(blocks_meta_t *blocks) {

    for (size_t i = 0; i < 62; i++) {
        int64_t block_id= blocks_alloc(blocks);
        assert(block_id != (size_t)-1);
        printf("Allocated block id: %lu\n", block_id);
    }
}

#include <stdlib.h> // 用于随机数生成
#include <time.h>   // 用于随机数种子
void test_free(blocks_meta_t *blocks) {
    int64_t block_id = blocks_alloc(blocks);
    for(int i=0;i<10;i++){
         block_id = blocks_alloc(blocks);
         LOG("alloc block %zu ",block_id);
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

void help(){
    printf("Usage: ./block_test <test_type>\n");
    printf("  test_type:\n");
    printf("    1 - Run test_alloc\n");
    printf("    2 - Run test_free\n");
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return 0;
    }
       

    int test_type = atoi(argv[1]); // 将 argv[1] 转换为整数

    uint8_t pool[1024 * 4];
    blocks_meta_t blocks;
    blocks_init(pool,1024 * 4, 64, &blocks);

    switch (test_type) {
        case 1:
            printf("Running test_alloc...\n");
            test_alloc(&blocks);
            break;
        case 2:
            printf("Running test_free...\n");
            test_free(&blocks);
            break;
        default:
            help();
            break;
    }

    return 0;
}