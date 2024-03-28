//filesystem_structs.h
#ifndef FILESYSTEM_STRUCTS_H
#define FILESYSTEM_STRUCTS_H
// 假设分区大小为10MB，为了简化示例
#define PARTITION_SIZE 1024 * 1024 * 10
// 块大小设定为512字节
#define BLOCK_SIZE 512
// 元数据占用的块数
#define META_BLOCKS 1
#define TOTAL_BLOCKS (PARTITION_SIZE / BLOCK_SIZE)
#define BITMAP_SIZE (TOTAL_BLOCKS / 8) // 每个位代表一个块，8位为1字节

#include <stdint.h>
#include <time.h>

// 文件元数据结构
typedef struct FileBlock {
    uint32_t blockNumber; // 当前块的块号
    struct FileBlock* next; // 指向下一个块的指针
} FileBlock;

typedef struct {
    char fileName[256];     // 文件名
    uint32_t fileSize;      // 文件大小（字节）
    FileBlock* firstBlock;  // 指向第一个文件块的指针
    // 其他元数据，如权限、创建和修改日期等
    time_t created;         // 创建日期
    time_t modified;        // 修改日期
} FileMeta;

typedef struct DirectoryEntry {
    char entryName[256]; // 目录项名称，可以是文件或子目录的名称
    struct DirectoryEntry* next; // 指向同一目录下一个目录项的指针
    FileMeta* fileMeta; // 指向文件元数据的指针，如果这是一个文件的目录项
    struct DirectoryEntry* child; // 指向子目录的指针，如果这是一个目录的目录项
} DirectoryEntry;

typedef struct {
    uint32_t block_size;    // 块大小
    uint32_t total_blocks;  // 总块数
    uint32_t free_blocks;   // 空闲块数，便于快速查看空闲块总数
    uint8_t block_bitmap[BITMAP_SIZE]; // 空闲块的位图
} FileSystemMeta;

typedef struct {
    DirectoryEntry* root; // 指向根目录的指针
    FileSystemMeta meta;  // 文件系统的元数据
} FileSystem;

// 文件系统元数据结构


// 文件类型定义
typedef struct {
    int fd;                  // 文件描述符
    FileMeta *meta;          // 指向文件元数据的指针
    FileBlock *currentBlock; // 当前块
    uint32_t currentPosition; // 当前在块中的位置
} file;

extern FileSystem fs;
#endif // FILESYSTEM_STRUCTS_H
