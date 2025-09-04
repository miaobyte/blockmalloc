/*
block_malloc 是一个专为管理固定大小的内存块（blocks）而设计的内存分配器。
它模拟了简单的内存池机制，支持高效的分配和释放操作，支持线程安全，保持api简洁性
*/

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

/**
 * 初始化块管理器元数据。
 * 检查总内存大小是否至少能容纳一个block_t结构体。
 * 初始化所有字段为默认值：块计数为0，空闲链表为空，锁为未锁定.
 */
int blocks_init(blocks_meta_t *meta, const uint64_t total_size, const uint64_t block_size);

/**
 * 分配一个块。
 * 检查空闲链表：若有空闲块则重用，否则检查内存是否足够分配新块。
 * 更新元数据计数和块状态，返回块block_id或-1（失败）。
 */
int64_t blocks_alloc(blocks_meta_t *meta, void *block_start);
/**
 * 释放一个块。
 * 验证块ID有效性，检查块是否已分配。
 * 更新块状态并将其添加回空闲链表，更新已用块计数.
 */
void blocks_free(blocks_meta_t *meta, void *block_start, const uint64_t block_id);

/*
辅助计算函数
计算对应块id，相对block_start的bytes偏移量
*/
int64_t block_offset(const blocks_meta_t *meta, const uint64_t block_id);

/*
 * 辅助计算函数
 * 计算对应块id的data起始地址，相对block_start的bytes偏移量
 */
int64_t block_data_offset(const blocks_meta_t *meta, const uint64_t block_id);

/*
 * 辅助计算函数
 * 从block相对block_start的bytes偏移量，计算对应块id
 */
int64_t block_id_byblockoffset(const blocks_meta_t *meta, const uint64_t block_offset);

/*
 * 辅助计算函数
 * 从blockdata相对block_start的bytes偏移量，计算对应块id
 */
int64_t block_id_bydataoffset(const blocks_meta_t *meta, const uint64_t data_offset);

#endif // BLOCK_MALLOC_H