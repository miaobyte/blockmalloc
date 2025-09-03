#ifndef BLOCK_MALLOC_H
#define BLOCK_MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

/*
内存布局图（meta 和 block 区域地址独立）：

+-----------------------+   <- meta 区起始地址 (meta_start)
| blocks_meta_t         |   <- sizeof(blocks_meta_t)
|-----------------------|
| total_size            |   uint64_t
| block_size            |   uint64_t
| total_blocks          |   uint64_t
| used_blocks           |   uint64_t
| free_next_id          |   int64_t
| lock                  |   _Atomic int64_t
+-----------------------+





+-----------------------+   <- block 区起始地址 (block_start)
| block 0              |   <- 每个 block 的大小为 block_size
|-----------------------|
| block_t              |   <- sizeof(block_t)
|   used               |   uint8_t (1 bit)
|   free_next_id       |   int64_t (63 bits)
|-----------------------|
| 用户数据区           |   剩余空间 (block_size - sizeof(block_t))
+-----------------------+
| block 1              |
|-----------------------|
| block_t              |
|   used               |
|   free_next_id       |
|-----------------------|
| 用户数据区           |
+-----------------------+
| ...                  |
+-----------------------+
| block (total_blocks-1)|
|-----------------------|
| block_t              |
|   used               |
|   free_next_id       |
|-----------------------|
| 用户数据区           |
+-----------------------+

*/

typedef struct
{
    uint64_t total_size;   // 总内存大小，不可变
    uint64_t block_size;   // 每个块的大小
    uint64_t total_blocks; // 总块数
    uint64_t used_blocks;  // 已使用的块数
    int64_t free_next_id;  // 首个空闲块的id,if no free block, this is -1;
    _Atomic int64_t lock;  // 原子锁，用于多线程同步
} blocks_meta_t;

int blocks_init(blocks_meta_t *meta, const uint64_t total_size, const uint64_t block_size);
int64_t blocks_alloc(blocks_meta_t *meta, void *block_start);
void blocks_free(blocks_meta_t *meta, void *block_start, const uint64_t block_id);

int64_t block_offset(const blocks_meta_t *meta, const uint64_t block_id);
int64_t block_data_offset(const blocks_meta_t *meta, const uint64_t block_id);
int64_t block_id_byblockoffset(const blocks_meta_t *meta, const uint64_t block_offset);
int64_t block_id_bydataoffset(const blocks_meta_t *meta, const uint64_t data_offset);

#endif // BLOCK_MALLOC_H