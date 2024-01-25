#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 100
#define MAX_BLOCKS 100  // 添加这行，定义磁盘块的最大数量

typedef struct {
    char fileName[MAX_FILENAME_LENGTH];
    int blocks[MAX_BLOCKS]; // 每个文件可以占用的块的数组
    int size;
    int position;
} file;

extern file fileIndex[MAX_FILES]; // 文件索引
extern int fileCount;             // 文件数量
extern int freeBlocks[MAX_BLOCKS]; // 空闲块数组

// 函数声明
int myFormat(char *partitionName);
file *myOpen(char* fileName);
// ...其他函数声明...

#endif // FILE_OPERATIONS_H
