//bitmap.c
// 引入所需的头文件
#include "bitmap.h"  // 包含位图操作的相关函数和定义
#include <stdio.h>  // 标准输入输出库，用于文件操作等
#include "filesystem_api.h"  // 包含文件系统API相关的定义

// 检查指定块是否空闲
int isBlockFree(FileSystemMeta* meta, uint32_t blockNumber) {
    uint32_t byteIndex = blockNumber / 8;  // 计算块号对应的字节索引（每个字节8位）
    uint32_t bitIndex = blockNumber % 8;  // 计算块号在字节中的位索引
    // 使用位掩码检查相应位是否为1（空闲）。如果是，返回非零值（真），否则返回0（假）。
    return (meta->block_bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

// 保存位图到分区文件
int saveBitmap(const FileSystemMeta* meta, const char* partitionName) {
    FILE* partitionFile = fopen(partitionName, "r+b");  // 以读/写模式打开分区文件
    if (!partitionFile) {
        perror("Failed to open partition file for updating");  // 打开文件失败时打印错误信息
        return -1;
    }

    // 将文件指针移动到位图应该存放的位置，即跳过文件系统元数据
    if (fseek(partitionFile, sizeof(FileSystemMeta), SEEK_SET) != 0) {
        perror("Failed to seek to the bitmap position");  // 定位失败时打印错误信息
        fclose(partitionFile);  // 关闭文件
        return -1;
    }

    // 将更新后的位图写入文件
    if (fwrite(meta->block_bitmap, 1, BITMAP_SIZE, partitionFile) != BITMAP_SIZE) {
        perror("Failed to write updated block bitmap to partition");  // 写入失败时打印错误信息
        fclose(partitionFile);  // 关闭文件
        return -1;
    }

    fclose(partitionFile);  // 关闭文件
    return 0;  // 写入成功，返回0
}

// 将指定块标记为已使用
void setBlockUsed(FileSystemMeta* meta, uint32_t blockNumber, const char* partitionName) {
    uint32_t byteIndex = blockNumber / 8;  // 计算字节索引
    uint32_t bitIndex = blockNumber % 8;  // 计算位索引
    // 使用位掩码将相应位清零，标记为已使用
    meta->block_bitmap[byteIndex] &= ~(1 << bitIndex);
    meta->free_blocks--;  // 空闲块数减一
    saveBitmap(meta, partitionName);  // 保存更新后的位图
}

// 将指定块标记为空闲
void setBlockFree(FileSystemMeta* meta, uint32_t blockNumber, const char* partitionName) {
    uint32_t byteIndex = blockNumber / 8;  // 计算字节索引
    uint32_t bitIndex = blockNumber % 8;  // 计算位索引
    // 使用位掩码将相应位置一，标记为空闲
    meta->block_bitmap[byteIndex] |= (1 << bitIndex);
    meta->free_blocks++;  // 空闲块数加一
    saveBitmap(meta, partitionName);  // 保存更新后的位图
}

// 分配一个空闲块
uint32_t allocateBlock(FileSystemMeta* meta, const char* partitionName) {
    // 遍历所有块，寻找空闲块
    for (uint32_t i = 0; i < meta->total_blocks; i++) {
        if (isBlockFree(meta, i)) {  // 如果找到空闲块
            setBlockUsed(meta, i, partitionName);  // 标记为已使用
            return i;  // 返回块号
        }
    }
    return (uint32_t)-1;  // 如果没有找到空闲块，返回-1
}
