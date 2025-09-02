
#include "block_malloc.h"
#include "logutil.h"
/*
下一步优化点

block_t 结构体中的 id 字段可以考虑去掉，因为它可以通过计算块的起始地址来推导出来，减少内存开销。
*/
typedef struct
{
    uint8_t used:1;         // 0: free, 1: used
    int64_t free_next_id:63; // -1,or id;
} __attribute__((packed)) block_t;

static void spin_lock(_Atomic int64_t *lock)
{
    while (atomic_exchange(lock, 1))
    {
        // 自旋等待，直到锁被释放
    }
}

static void spin_unlock(_Atomic int64_t *lock)
{
    atomic_store(lock, 0); // 释放锁
}

int blocks_init(blocks_meta_t *meta,const uint64_t total_size, const uint64_t block_size)
{
    if (total_size < sizeof(block_t))
    {
        LOG("Total size %zu is too small for blocks_meta_t and block_t structures", total_size);
        return -1;
    }
    *meta = (blocks_meta_t){
        .total_size = total_size,
        .block_size = block_size,
        .total_blocks = 0,
        .used_blocks = 0,
        .free_next_id = -1, // 初始化为 -1，表示没有空闲块，需要新增分配
        .lock = 0,          // 初始化锁为未锁定状态
    };
    return 0;
}
 
 int64_t block_offset(const blocks_meta_t *meta, const uint64_t block_id)
{
    if (block_id >= (meta->total_blocks))
    {
        LOG("block id %zu out of range", block_id);
        return -1; // Return -1 for invalid id
    }
    int64_t offset =   (sizeof(block_t) + meta->block_size) * block_id;
    return offset;
}
 int64_t block_data_offset(const blocks_meta_t *meta, const uint64_t block_id)
{
    int64_t offset = block_offset(meta, block_id);
    if (offset == -1)
        return -1;
    return offset + sizeof(block_t);
}

 int64_t block_id_byblockoffset(const blocks_meta_t *meta,const uint64_t block_offset)
{
    uint64_t block_size = meta->block_size + sizeof(block_t);
    if (block_offset % block_size != 0) {
        LOG("block_offset %zu is not aligned with block_t size %zu", block_offset, sizeof(block_t));
        return -1;
    }
    uint64_t block_id = block_offset / block_size;
    return block_id;
}

 int64_t block_id_bydataoffset(const blocks_meta_t *meta, const uint64_t  data_offset)
{
    return block_id_byblockoffset(meta, data_offset - sizeof(block_t));
}

int64_t blocks_alloc(blocks_meta_t *meta, void *block_start)
{
    spin_lock(&meta->lock);
    if (meta->free_next_id == -1)
    {
        uint64_t totalused_size = meta->total_blocks * meta->block_size;
        if (totalused_size + meta->block_size > meta->total_size)
        {
            LOG("Out of memory. %zu(totalused_size)= %zu(blocks_meta_t)+ %zu(total_blocks)*%zu(block_size),when total_size %zu",
                totalused_size, sizeof(blocks_meta_t), meta->total_blocks, meta->block_size, meta->total_size);
            spin_unlock(&meta->lock);
            return -1;
        }
        else
        {
            meta->total_blocks++;
            meta->used_blocks++;

            uint64_t block_id = meta->total_blocks - 1;
            int64_t b_offset = block_offset(meta, block_id);
            block_t *block = (block_t*)(block_start + b_offset);
            *block = (block_t){
                .used = 1,
                .free_next_id = -1,
            };

            LOG("append new block %zu,meta usage: %zu / %zu", block_id, meta->used_blocks, meta->total_blocks);
            spin_unlock(&meta->lock);
            return block_id;
        }
    }
    else
    {
        uint64_t free_id = meta->free_next_id;
        int64_t b_offset = block_offset(meta, free_id);
        block_t *free_block = (block_t*)(block_start + b_offset);
        meta->free_next_id = free_block->free_next_id;
        meta->used_blocks++;

        free_block->used = 1;
        free_block->free_next_id = -1;

        LOG("Reusing free block %zu, Used meta: %zu/%zu",
            free_id, meta->used_blocks, meta->total_blocks);
        spin_unlock(&meta->lock);
        return free_id;
    }
}

void blocks_free(blocks_meta_t *meta, void *block_start, const uint64_t block_id)
{
    spin_lock(&meta->lock);
    if (block_id >= meta->total_blocks)
    {
        LOG("block id %zu out of range", block_id);
        spin_unlock(&(meta->lock));
        return;
    }
    int64_t b_offset = block_offset(meta, block_id);
    block_t *free_block = (block_t*)(block_start + b_offset);

    switch (free_block->used)
    {
    case 0:
        LOG("block id %zu already free", block_id);
        break;
    case 1:
        free_block->used = 0;
        free_block->free_next_id = meta->free_next_id;

        meta->free_next_id = block_id;
        meta->used_blocks--;
        break;
    default:
        LOG("block id %zu status invalid %d", block_id, free_block->used);
        break;
    }
    LOG("block id %zu ->free", block_id);
    spin_unlock(&(meta->lock));
}