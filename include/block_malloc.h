#ifndef BLOCK_MALLOC_H
#define BLOCK_MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

/*
__attribute__((packed)) 暂时不用
*/
typedef struct
{
    uint64_t total_size;   // 总内存大小，不可变
    uint64_t block_size;   // 每个块的大小
    uint64_t total_blocks; // 总块数
    uint64_t used_blocks;  // 已使用的块数
    int64_t free_next_id; // 首个空闲块的id,if no free block, this is -1;
    void *start; // 指向块内存起始位置
    _Atomic int64_t lock; // 原子锁，用于多线程同步
} blocks_meta_t;


int blocks_init(void *block_start,const uint64_t total_size, const uint64_t block_size,blocks_meta_t* blocks );


int64_t blocks_alloc(blocks_meta_t* meta);
void blocks_free(blocks_meta_t *meta,const uint64_t block_id);

void *block_ptr(const blocks_meta_t* meta,const uint64_t block_id);
void *block_data(const blocks_meta_t* meta,const uint64_t block_id);
int64_t block_bydata(const void* block_data);

#endif // BLOCK_MALLOC_H