// file_operations.c
#include "file_operations.h"
#include <stdio.h>
#include <string.h>

file fileIndex[MAX_FILES];
int fileCount = 0;
int freeBlocks[MAX_BLOCKS] = {0}; // 初始化所有块为空闲

int myFormat(char *partitionName) {
    FILE *file;

    // 打开或创建文件
    file = fopen(partitionName, "wb");
    if (file == NULL) {
        return -1; // 文件打开失败
    }

    // 初始化分区（示例：写入一些初始化数据或元数据）
    const char *initData = "This is the virtual partition for my file system.";
    fwrite(initData, 1, strlen(initData), file);

    // 关闭文件
    fclose(file);

    return 0; // 格式化成功
}

// 实现myOpen函数：在虚拟分区中打开或创建一个文件。
file * myOpen(char* fileName) {
    // 检查文件是否已存在于索引中
    for (int i = 0; i < fileCount; i++) {
        if (strcmp(fileIndex[i].fileName, fileName) == 0) {
            // 文件已存在
            return &fileIndex[i];
        }
    }

    if (fileCount < MAX_FILES) {
        strncpy(fileIndex[fileCount].fileName, fileName, MAX_FILENAME_LENGTH);

        // 分配第一个空闲块
        for (int i = 0; i < MAX_BLOCKS; i++) {
            if (freeBlocks[i]) {
                fileIndex[fileCount].blocks[0] = i; // 分配第一个空闲块
                freeBlocks[i] = 0; // 标记为已占用
                break;
            }
        }

        fileIndex[fileCount].size = 0;
        fileIndex[fileCount].position = 0;
        fileCount++;
        return &fileIndex[fileCount - 1];
    } else {
        // 文件索引已满
        return NULL;
    }
}

int myWrite(file* f, void* buffer, int nBytes) {
    // Open the partition
    FILE *partition = fopen("partition.bin", "r+b");
    if (partition == NULL) {
        return -1; // Error opening partition
    }

    // Calculate the offset to seek to the start of the block
    fseek(partition, f->blocks[f->position] * BLOCK_SIZE, SEEK_SET);

    // Write data and update the file structure
    int bytesWritten = fwrite(buffer, 1, nBytes, partition);
    f->size += bytesWritten;
    f->position += bytesWritten;

    // Close the partition
    fclose(partition);

    return bytesWritten; // Return the number of bytes written
}

// Function to read data from a file in the virtual partition
int myRead(file* f, void* buffer, int nBytes) {
    // Open the partition
    FILE *partition = fopen("partition.bin", "rb");
    if (partition == NULL) {
        return -1; // Error opening partition
    }

    // Check if we are trying to read more bytes than the file contains
    if(nBytes > f->size - f->position) {
        nBytes = f->size - f->position; // Read only up to the end of the file
    }

    // Calculate the offset to seek to the current read position
    fseek(partition, f->blocks[f->position] * BLOCK_SIZE, SEEK_SET);

    // Read data into the buffer
    int bytesRead = fread(buffer, 1, nBytes, partition);
    f->position += bytesRead; // Update the file's current position

    // Close the partition
    fclose(partition);

    return bytesRead; // Return the number of bytes read
}
// 实现mySeek函数：改变文件的当前读/写位置。