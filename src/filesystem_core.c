//filesystem_core.c
#include "filesystem_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include <fcntl.h>


// 假设分区大小为10MB，为了简化示例
#define PARTITION_SIZE 1024 * 1024 * 10
// 块大小设定为512字节
#define BLOCK_SIZE 512
// 元数据占用的块数
#define META_BLOCKS 1

FileSystem fs;

int myFormat(const char* partitionName) {
    FILE* partitionFile = fopen(partitionName, "wb");
    if (!partitionFile) {
        perror("Failed to open partition file");
        return -1;
    }

    // 初始化文件系统元数据
    FileSystemMeta meta;
    meta.block_size = BLOCK_SIZE;
    meta.total_blocks = PARTITION_SIZE / BLOCK_SIZE;
    // 直接计算空闲块数量，考虑到元数据占用的块和位图占用的块
    meta.free_blocks = meta.total_blocks - META_BLOCKS - (BITMAP_SIZE / BLOCK_SIZE + ((BITMAP_SIZE % BLOCK_SIZE) ? 1 : 0));

    // 写入文件系统元数据到分区文件的开头
    if (fwrite(&meta, sizeof(FileSystemMeta), 1, partitionFile) != 1) {
        perror("Failed to write file system meta to partition");
        fclose(partitionFile);
        return -1;
    }

    // 初始化空间管理位图
    uint8_t* bitmap = (uint8_t*)malloc(BITMAP_SIZE);
    memset(bitmap, 0xFF, BITMAP_SIZE); // 将所有位初始化为1，表示空闲

    // 将位图写入分区文件，紧跟在文件系统元数据后
    if (fwrite(bitmap, 1, BITMAP_SIZE, partitionFile) != BITMAP_SIZE) {
        perror("Failed to write block bitmap to partition");
        fclose(partitionFile);
        free(bitmap);
        return -1;
    }
    free(bitmap); // 释放位图缓冲区

    // 填充剩余的分区空间以确保分区文件达到预设大小
    fseek(partitionFile, PARTITION_SIZE - 1, SEEK_SET);
    fputc('\0', partitionFile); // 在文件的最后位置写入一个字节

    fclose(partitionFile);
    return 0;
}

int loadBitmap(FileSystemMeta* meta, const char* partitionName) {
    // 打开分区文件
    FILE* partitionFile = fopen(partitionName, "rb");
    if (!partitionFile) {
        perror("Failed to open partition file for reading");
        return -1;
    }

    // 首先读取文件系统元数据
    if (fread(meta, sizeof(FileSystemMeta), 1, partitionFile) != 1) {
        perror("Failed to read file system meta from partition");
        fclose(partitionFile);
        return -1;
    }

    // 根据读取的元数据，加载位图到内存
    // 确保meta->block_bitmap有足够的空间
    if (fread(meta->block_bitmap, 1, BITMAP_SIZE, partitionFile) != BITMAP_SIZE) {
        perror("Failed to read block bitmap from partition");
        fclose(partitionFile);
        return -1;
    }

    fclose(partitionFile);
    return 0;
}

void initFileSystem( const char* partitionName) {
    // 分配内存并初始化根目录等
    void cleanupFileSystem();
    fs.root = (DirectoryEntry*)malloc(sizeof(DirectoryEntry));
    strcpy(fs.root->entryName, "/");
    fs.root->next = NULL;
    fs.root->fileMeta = NULL;
    fs.root->child = NULL;

    // 加载空闲块位图到内存
    if (loadBitmap(&(fs.meta), partitionName) != 0) {
        fprintf(stderr, "Error loading the bitmap.\n");
        // 处理错误，可能需要清理已分配的资源
    }
}
FileBlock* allocateFileBlock(FileMeta* meta, uint32_t blockNumber) {
    // 创建新的文件块
    FileBlock* newBlock = (FileBlock*)malloc(sizeof(FileBlock));
    newBlock->blockNumber = blockNumber;
    newBlock->next = NULL;

    // 将新块添加到文件的块列表中
    if (meta->firstBlock == NULL) {
        // 这是文件的第一个块
        meta->firstBlock = newBlock;
    } else {
        // 找到列表的最后一个块并链接新块
        FileBlock* current = meta->firstBlock;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newBlock;
    }

    return newBlock;
}
DirectoryEntry* createEntry(DirectoryEntry* parent, const char* name, int isDirectory, const char* partitionName) {
    DirectoryEntry* newEntry = (DirectoryEntry*)malloc(sizeof(DirectoryEntry));
    if (newEntry == NULL) {
        fprintf(stderr, "Memory allocation failed for directory entry.\n");
        return NULL;
    }

    strcpy(newEntry->entryName, name);
    newEntry->next = parent->child;
    if (isDirectory) {
        newEntry->child = NULL;
        newEntry->fileMeta = NULL; // 目录没有文件元数据
    } else {
        // 对于文件，初始化文件元数据并分配起始块
        newEntry->child = NULL; // 文件没有子目录
        newEntry->fileMeta = (FileMeta*)malloc(sizeof(FileMeta));
        if (newEntry->fileMeta == NULL) {
            fprintf(stderr, "Memory allocation failed for file metadata.\n");
            free(newEntry); // 释放之前分配的内存
            return NULL;
        }

        strcpy(newEntry->fileMeta->fileName, name);
        newEntry->fileMeta->fileSize = 0; // 新文件的初始大小为0
        newEntry->fileMeta->firstBlock = NULL; // 初始化第一个块指针

        // 为新文件分配一个起始块
        uint32_t blockNumber = allocateBlock(&fs.meta, partitionName);
        if (blockNumber != (uint32_t)-1) {
            // 成功分配块，更新文件元数据
            FileBlock* newBlock = allocateFileBlock(newEntry->fileMeta, blockNumber);
            newEntry->fileMeta->firstBlock = newBlock;
        } else {
            // 处理块分配失败的情况
            fprintf(stderr, "Failed to allocate a block for the new file.\n");
            free(newEntry->fileMeta); // 清理文件元数据分配的内存
            free(newEntry); // 清理目录项分配的内存
            return NULL;
        }
    }
    parent->child = newEntry; // 将新条目添加到父目录的子项列表
    return newEntry;
}

DirectoryEntry* findEntry(DirectoryEntry* root, const char* path) {
    if (root == NULL || path == NULL) return NULL;

    // 如果路径是根目录，直接返回根
    if (strcmp(path, "/") == 0) return root;

    DirectoryEntry* current = root->child; // 从根目录的第一个子节点开始
    char* pathCopy = strdup(path); // 复制路径字符串，因为strtok会修改原字符串
    char* token = strtok(pathCopy, "/"); // 按"/"分割路径

    while (token != NULL && current != NULL) {
        DirectoryEntry* found = NULL;
        // 遍历当前目录的所有子节点
        while (current != NULL) {
            if (strcmp(current->entryName, token) == 0) {
                found = current;
                break;
            }
            current = current->next;
        }

        if (found == NULL) {
            // 没有找到匹配的子目录或文件
            free(pathCopy);
            return NULL;
        }

        // 准备查找下一级目录
        token = strtok(NULL, "/");
        if (token != NULL) {
            // 如果还有更深层的目录，继续遍历
            current = found->child;
        } else {
            // 找到目标文件或目录
            free(pathCopy);
            return found;
        }
    }

    free(pathCopy);
    return NULL; // 路径无效或找不到目标
}



file* myOpen(const char* partitionName, const char* fileName) {
    DirectoryEntry* parent = fs.root;
    DirectoryEntry* entry = findEntry(parent, fileName);

    if (entry == NULL) {
        // 文件不存在，尝试创建它
        entry = createEntry( parent, fileName, 0, partitionName); // 0 表示文件
        if (entry == NULL) {
            fprintf(stderr, "Failed to create file: %s\n", fileName);
            return NULL;
        }
    }

    // 分配并初始化 file 结构
    file* f = (file*)malloc(sizeof(file));
    if (!f) {
        fprintf(stderr, "Memory allocation failed for file structure.\n");
        return NULL;
    }

    // 初始化 file 结构
    f->meta = entry->fileMeta;
    f->currentBlock = entry->fileMeta->firstBlock; // 对于新文件，这可能是 NULL
    f->currentPosition = 0;

    // 打开或创建底层的物理文件
    // 注意：此处应确保你有机制来管理文件描述符或者操作文件
    int fd = open(partitionName, O_RDWR); // 以读写方式打开映像文件
    if (fd == -1) {
        fprintf(stderr, "Failed to open partition file: %s\n", partitionName);
        free(f);
        return NULL;
    }
    f->fd = fd; // 保存文件描述符

    return f;
}



int myWrite(file* f,  void* buffer, int nBytes, const char* partitionName ) {
    if (f == NULL || buffer == NULL || nBytes <= 0 || f->fd < 0) return -1;

    char* buf = (char*)buffer;
    int bytesWritten = 0;

    while (bytesWritten < nBytes) {
        if (!f->currentBlock || f->currentPosition == BLOCK_SIZE) {
            // 当前块为空或已满，分配新块
            uint32_t newBlockNum = allocateBlock(&fs.meta,partitionName);
            if (newBlockNum == (uint32_t)-1) {
                // 无空闲块可分配
                break;
            }

            FileBlock* newBlock = allocateFileBlock(f->meta, newBlockNum);
            if (!f->meta->firstBlock) {
                f->meta->firstBlock = newBlock; // 新块成为第一个块
            }
            if (f->currentBlock) {
                f->currentBlock->next = newBlock; // 连接到当前块链
            }
            f->currentBlock = newBlock; // 更新当前块指针
            f->currentPosition = 0; // 重置块内偏移
        }

        // 计算本次写入量
        int remaining = BLOCK_SIZE - f->currentPosition;
        int toWrite = nBytes - bytesWritten < remaining ? nBytes - bytesWritten : remaining;

        // 写入操作
        off_t offset = (off_t)f->currentBlock->blockNumber * BLOCK_SIZE + f->currentPosition;
        if (lseek(f->fd, offset, SEEK_SET) < 0) {
            perror("Lseek error");
            break;
        }
        ssize_t written = write(f->fd, buf + bytesWritten, toWrite);
        if (written < 0) {
            perror("Write error");
            break;
        }

        // 更新写入状态
        bytesWritten += written;
        f->currentPosition += written;
    }

    // 更新文件大小，如果需要
    if ((off_t)f->meta->fileSize < (off_t)f->currentBlock->blockNumber * BLOCK_SIZE + f->currentPosition) {
        f->meta->fileSize = f->currentBlock->blockNumber * BLOCK_SIZE + f->currentPosition;
    }

    return bytesWritten;
}


int myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) return -1;

    char* buf = (char*)buffer;
    int bytesRead = 0;

    while (nBytes > 0 && f->currentBlock != NULL) {
        long blockOffset = (long)f->currentBlock->blockNumber * BLOCK_SIZE;
        int offsetInBlock = f->currentPosition % BLOCK_SIZE;
        int spaceInBlock = BLOCK_SIZE - offsetInBlock;
        int toRead = (nBytes < spaceInBlock) ? nBytes : spaceInBlock;

        // Adjust read length if it exceeds the remaining file size
        long remainingFileSize = f->meta->fileSize - (blockOffset + offsetInBlock);
        toRead = (toRead > remainingFileSize) ? remainingFileSize : toRead;
        if (toRead <= 0) break; // No more data to read

        // Position the file descriptor and perform the read operation
        if (lseek(f->fd, blockOffset + offsetInBlock, SEEK_SET) < 0) {
            perror("Lseek error");
            break;
        }
        ssize_t readCount = read(f->fd, buf + bytesRead, toRead);
        if (readCount < 0) {
            perror("Read error");
            break;
        }

        // Update the counters and positions
        bytesRead += readCount;
        f->currentPosition += readCount;
        nBytes -= readCount;

        // Check if need to move to the next block
        if (f->currentPosition >= BLOCK_SIZE && readCount > 0) {
            f->currentPosition = 0; // Reset block position
            f->currentBlock = f->currentBlock->next; // Move to next block
        }
    }

    return bytesRead;
}


void mySeek(file* f, int offset, int whence) {
    if (!f || !f->meta) return;

    long newPosition = 0;

    // 根据基点计算新的逻辑位置
    switch (whence) {
        case SEEK_SET:
            newPosition = offset;
            break;
        case SEEK_CUR:
            newPosition = f->currentPosition + offset;
            break;
        case SEEK_END:
            newPosition = f->meta->fileSize + offset;
            break;
        default:
            return; // 不支持的whence类型
    }

    // 确保新位置在文件大小范围内
    if (newPosition < 0) newPosition = 0;
    if (newPosition > f->meta->fileSize) newPosition = f->meta->fileSize;

    // 重新定位到新的逻辑位置
    f->currentPosition = newPosition;
    long blockOffset = newPosition / BLOCK_SIZE;
    long intraBlockOffset = newPosition % BLOCK_SIZE;

    // 重新定位currentBlock到对应的块
    f->currentBlock = f->meta->firstBlock;
    for (long i = 0; i < blockOffset && f->currentBlock != NULL; i++) {
        f->currentBlock = f->currentBlock->next;
    }

    // 更新当前块内的位置
    f->currentPosition = intraBlockOffset;
}

void myClose(file* f) {
    if (!f) return;

    // 释放与文件关联的所有动态分配的内存
    FileBlock* block = f->meta->firstBlock;
    while (block) {
        FileBlock* temp = block;
        block = block->next;
        free(temp);  // 释放每个文件块结构
    }

    free(f->meta);  // 如果文件元数据是动态分配的，也需要释放
    close(f->fd);   // 关闭文件描述符
    free(f);        // 最后释放文件结构自身
}

void deleteEntry(DirectoryEntry* entry) {
    if (!entry) return;

    // 如果有子目录项或文件，递归删除
    while (entry->child) {
        DirectoryEntry* temp = entry->child;
        entry->child = entry->child->next;
        deleteEntry(temp);
    }

    // 释放与目录项关联的文件元数据和文件块
    if (entry->fileMeta) {
        FileBlock* block = entry->fileMeta->firstBlock;
        while (block) {
            FileBlock* temp = block;
            block = block->next;
            free(temp);
        }
        free(entry->fileMeta);
    }

    free(entry);
}

void cleanupFileSystem() {
    if (fs.root) {
        deleteEntry(fs.root); // 释放目录项及其子项的内存
        fs.root = NULL;
    }

    // 现在 block_bitmap 是一个指针，可以释放并设置为 NULL
    // free(fs.meta.block_bitmap);
    memset(fs.meta.block_bitmap, 0, sizeof(fs.meta.block_bitmap));

}
