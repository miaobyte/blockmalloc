#include "block_malloc.h"
#include "logutil.h"
/*
下一步优化点

block_t 结构体中的 id 字段可以考虑去掉，因为它可以通过计算块的起始地址来推导出来，减少内存开销。    
*/
typedef struct
{
    uint64_t id;
    uint8_t used;        // 0: free, 1: used
    int64_t free_next_id; // -1,or id;
} __attribute__((packed)) block_t;


int blocks_init(void *block_start,const uint64_t total_size, const uint64_t block_size,blocks_meta_t* meta )
{
    if (total_size < sizeof(block_t))
    {
        LOG("Total size %zu is too small for blocks_meta_t and block_t structures", total_size);
        return -1;
    }
    *meta=(blocks_meta_t){
        .total_size = total_size,
        .block_size = block_size,
        .total_blocks = 0,
        .used_blocks = 0,
        .free_next_id = -1, // 初始化为 -1，表示没有空闲块，需要新增分配
        .start = block_start,
    };
    return 0;
}

void *block_ptr(const blocks_meta_t* meta,const uint64_t id)
{
    if (id >= (meta->total_blocks))
    {
        LOG("block id %zu out of range", id);
        return NULL; // Return NULL for invalid id
    }
    void *ptr =meta->start +(sizeof(block_t) + meta->block_size) * id;
    return ptr;
}
void *block_data(const blocks_meta_t* meta,const uint64_t id)
{
    void *ptr =block_ptr(meta,id);
    if(!ptr) return NULL;
    ptr += sizeof(block_t);
    return ptr;
}
int64_t block_bydata(const void* blockdata){
    void *ptr=(void*)blockdata - sizeof(block_t);
    block_t *block = (block_t *)ptr;
    if(!block) return -1;
    return block->id;;
}
int64_t blocks_alloc(blocks_meta_t* meta)
{
    if (meta->free_next_id == -1)
    {
        uint64_t totalused_size = meta->total_blocks * meta->block_size;
        if (totalused_size + meta->block_size > meta->total_size)
        {
            LOG("Out of memory. %zu(totalused_size)= %zu(blocks_meta_t)+ %zu(total_blocks)*%zu(block_size),when total_size %zu",
                totalused_size, sizeof(blocks_meta_t), meta->total_blocks, meta->block_size, meta->total_size);
            return -1;
        }
        else
        {
            meta->total_blocks++;
            meta->used_blocks++;

            block_t *block = block_ptr(meta, meta->total_blocks - 1);
            *block = (block_t){
                .id = meta->total_blocks - 1,
                .used = 1,
                .free_next_id = -1,
            };

            LOG("append new block %zu,meta usage: %zu / %zu",block->id, meta->used_blocks, meta->total_blocks);
            return block->id;
        }
    }
    uint64_t free_id = meta->free_next_id;
    block_t *free_block = block_ptr(meta,  meta->free_next_id);
    meta->free_next_id = free_block->free_next_id;
    meta->used_blocks++;

    free_block->used = 1;
    free_block->free_next_id =-1;

    LOG("Reusing free block %zu, Used meta: %zu/%zu",
        free_id, meta->used_blocks, meta->total_blocks);

    return free_block->id;
}

void blocks_free(blocks_meta_t *meta,const uint64_t id)
{
    LOG("block id %zu ->free", id);
    if (id >= meta->total_blocks)
    {
        LOG("block id %zu out of range", id);
        return;
    }
    block_t *free_block = block_ptr(meta, id);
    switch (free_block->used)
    {
    case 0:
        LOG("block id %zu already free", id);
        return;
    case 1:
        break;
    default:
        LOG("block id %zu status invalid %d", id, free_block->used);
        return;
    }
    free_block->used = 0;
    free_block->free_next_id = meta->free_next_id;
    meta->free_next_id = free_block->id;

    meta->used_blocks--;
}