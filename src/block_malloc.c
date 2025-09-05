
#include "block_malloc.h"
#include "logutil.h"
/*
下一步优化点

block_t 结构体中的 id 字段可以考虑去掉，因为它可以通过计算块的起始地址来推导出来，减少内存开销。
*/
typedef struct
{
    uint8_t used : 1;          // 0: free, 1: used
    int64_t free_next_id : 63; // -1,or id;
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

#define BLOCK_SIZE(meta) (sizeof(block_t) + (meta)->block_size)

int blocks_init(blocks_meta_t *meta, const uint64_t total_size, const uint64_t block_size)
{
    if (meta == NULL)
    {
        LOG("[ERROR] meta is NULL");
        return -1;
    }
    if (total_size < sizeof(block_t) + block_size)
    {
        LOG("[ERROR] total size %zu is too small for one block_tdata", total_size);
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
    int64_t offset =  block_id*BLOCK_SIZE(meta);
    return offset;
}
int64_t block_data_offset(const blocks_meta_t *meta, const uint64_t block_id)
{
    int64_t offset = block_offset(meta, block_id);
    if (offset == -1)
        return -1;
    return offset + sizeof(block_t);
}

int64_t block_id_byblockoffset(const blocks_meta_t *meta, const uint64_t block_offset)
{
    uint64_t block_size = BLOCK_SIZE(meta);
    if (block_offset % block_size != 0)
    {
        // 计算建议的正确偏移量（向下取整到最近的块边界）
        uint64_t suggested_offset = block_offset - (block_offset % block_size);
        LOG("[ERROR] block_offset %zu is not aligned with BLOCK_SIZE %zu, suggested offset: %zu",
            block_offset, block_size, suggested_offset);
        return -1;
    }
    uint64_t block_id = block_offset / block_size;
    return block_id;
}

int64_t block_id_bydataoffset(const blocks_meta_t *meta, const uint64_t data_offset)
{
    return block_id_byblockoffset(meta, data_offset - sizeof(block_t));
}

int64_t blocks_alloc(blocks_meta_t *meta, void *block_start)
{
    if (meta == NULL || block_start == NULL)
    {
        LOG("[ERROR] meta or block_start is NULL");
        return -1;
    }
    spin_lock(&meta->lock);
    if (meta->free_next_id == -1)
    {
        uint64_t totalused_size = block_offset(meta,meta->total_blocks);
        if (totalused_size + BLOCK_SIZE(meta)> meta->total_size)
        {
            LOG("[ERROR] out of memory,%zu(used_size)= %zu(total_blocks)*(sizeof(block_t)=%zu)+%zu(block_size)),when total_size %zu",
                totalused_size, meta->total_blocks,sizeof(block_t), meta->block_size, meta->total_size);
            spin_unlock(&meta->lock);
            return -1;
        }
        else
        {
            meta->total_blocks++;
            meta->used_blocks++;

            uint64_t block_id = meta->total_blocks - 1;
            int64_t b_offset = block_offset(meta, block_id);
            block_t *block = (block_t *)(block_start + b_offset);
            *block = (block_t){
                .used = 1,
                .free_next_id = -1,
            };

            LOG("[INFO] append block %zu,blocks usage: %zu/%zu", block_id, meta->used_blocks, meta->total_blocks);
            spin_unlock(&meta->lock);
            return block_id;
        }
    }
    else
    {
        uint64_t free_id = meta->free_next_id;
        int64_t b_offset = block_offset(meta, free_id);
        block_t *free_block = (block_t *)(block_start + b_offset);
        meta->free_next_id = free_block->free_next_id;
        meta->used_blocks++;

        free_block->used = 1;
        free_block->free_next_id = -1;

        LOG("[INFO] reusing block %zu, blocks usage: %zu/%zu",
            free_id, meta->used_blocks, meta->total_blocks);
        spin_unlock(&meta->lock);
        return free_id;
    }
}

void blocks_free(blocks_meta_t *meta, void *block_start, const uint64_t block_id)
{
    if (meta == NULL || block_start == NULL)
    {
        LOG("[ERROR] meta or block_start must not NULL");
        return;
    }
    spin_lock(&meta->lock);
    if (block_id >= meta->total_blocks)
    {
        LOG("[ERROR] block id %zu out of range", block_id);
        spin_unlock(&(meta->lock));
        return;
    }
    int64_t b_offset = block_offset(meta, block_id);
    block_t *free_block = (block_t *)(block_start + b_offset);

    switch (free_block->used)
    {
    case 0:
        LOG("[WARN] block id %zu already free", block_id);
        break;
    case 1:
        free_block->used = 0;
        free_block->free_next_id = meta->free_next_id;

        meta->free_next_id = block_id;
        meta->used_blocks--;
        break;
    default:
        LOG("[WARN] block id %zu status invalid %d,who make this?", block_id, free_block->used);
        break;
    }
    LOG("[INFO] block id %zu freed", block_id);
    spin_unlock(&(meta->lock));
}