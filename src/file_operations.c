#include "file_operations.h"
#include <fcntl.h>  // 包含open函数
#include <unistd.h> // 包含close、read、write和lseek函数
#include <string.h> // 包含strcpy等字符串操作函数

int myFormat(char *partitionName) {
    int fd = open(partitionName, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
        return -1; // 文件打开失败
    }

    const char *initData = "This is the virtual partition for my file system.";
    write(fd, initData, strlen(initData));

    close(fd);
    return 0; // 格式化成功
}

file *myOpen(char *fileName) {
    for (int i = 0; i < fileCount; i++) {
        if (strcmp(fileIndex[i].fileName, fileName) == 0) {
            // 文件已存在于文件索引中
            return &fileIndex[i];
        }
    }

    if (fileCount < MAX_FILES) {
        // 为新文件设置文件名和初始化参数
        strncpy(fileIndex[fileCount].fileName, fileName, MAX_FILENAME_LENGTH);
        fileIndex[fileCount].size = 0;
        fileIndex[fileCount].position = 0;
        // 这里可以添加更多初始化代码，如分配初始块等

        fileCount++;
        return &fileIndex[fileCount - 1];
    } else {
        // 文件索引已满
        return NULL;
    }
}

int myWrite(file *f, void *buffer, int nBytes) {
    int fd = open("partition.bin", O_WRONLY);
    if (fd == -1) {
        return -1; // Error opening partition
    }

    lseek(fd, f->blocks[f->position] * BLOCK_SIZE, SEEK_SET);
    int bytesWritten = write(fd, buffer, nBytes);

    f->size += bytesWritten;
    f->position += bytesWritten;

    close(fd);
    return bytesWritten;
}

int myRead(file *f, void *buffer, int nBytes) {
    int fd = open("partition.bin", O_RDONLY);
    if (fd == -1) {
        return -1; // Error opening partition
    }

    lseek(fd, f->blocks[f->position] * BLOCK_SIZE, SEEK_SET);
    int bytesRead = read(fd, buffer, nBytes);
    f->position += bytesRead;

    close(fd);
    return bytesRead;
}

void mySeek(file *f, int offset, int base) {
    switch (base) {
        case SEEK_SET:
            f->position = offset;
            break;
        case SEEK_CUR:
            f->position += offset;
            break;
        case SEEK_END:
            f->position = f->size + offset;
            break;
    }

    // 确保位置不超出文件范围
    if (f->position > f->size) {
        f->position = f->size;
    }
    if (f->position < 0) {
        f->position = 0;
    }
}
