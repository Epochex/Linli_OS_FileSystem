//bitmap.c
#include "bitmap.h"
#include <stdio.h>
#include "filesystem_api.h."
// 检查块是否空闲
int isBlockFree(FileSystemMeta* meta, uint32_t blockNumber) {
    uint32_t byteIndex = blockNumber / 8;
    uint32_t bitIndex = blockNumber % 8;
    return (meta->block_bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

int saveBitmap(const FileSystemMeta* meta, const char* partitionName) {
    FILE* partitionFile = fopen(partitionName, "r+b"); // 打开文件用于更新
    if (!partitionFile) {
        perror("Failed to open partition file for updating");
        return -1;
    }

    // 跳过文件系统元数据的位置
    if (fseek(partitionFile, sizeof(FileSystemMeta), SEEK_SET) != 0) {
        perror("Failed to seek to the bitmap position");
        fclose(partitionFile);
        return -1;
    }

    // 写入更新后的位图
    if (fwrite(meta->block_bitmap, 1, BITMAP_SIZE, partitionFile) != BITMAP_SIZE) {
        perror("Failed to write updated block bitmap to partition");
        fclose(partitionFile);
        return -1;
    }

    fclose(partitionFile);
    return 0;
}

// 设置块为已用
void setBlockUsed(FileSystemMeta* meta, uint32_t blockNumber, const char* partitionName) {
    uint32_t byteIndex = blockNumber / 8;
    uint32_t bitIndex = blockNumber % 8;
    meta->block_bitmap[byteIndex] &= ~(1 << bitIndex);
    meta->free_blocks--;
    saveBitmap(meta, partitionName);
}


// 设置块为空闲
void setBlockFree(FileSystemMeta* meta, uint32_t blockNumber, const char* partitionName) {
    uint32_t byteIndex = blockNumber / 8;
    uint32_t bitIndex = blockNumber % 8;
    meta->block_bitmap[byteIndex] |= (1 << bitIndex);
    meta->free_blocks++;
    saveBitmap(meta, partitionName);
}


// 分配一个空闲块
uint32_t allocateBlock(FileSystemMeta* meta, const char* partitionName) {
    for (uint32_t i = 0; i < meta->total_blocks; i++) {
        if (isBlockFree(meta, i)) {
            setBlockUsed(meta, i, partitionName);
            return i;
        }
    }
    return (uint32_t)-1; // 没有空闲块
}



