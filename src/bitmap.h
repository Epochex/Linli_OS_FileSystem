//bitmap.
#ifndef BITMAP_H
#define BITMAP_H

#include "filesystem_structs.h" // 确保包含了FileSystemMeta的定义

int isBlockFree(FileSystemMeta* meta, uint32_t blockNumber);
void setBlockUsed(FileSystemMeta* meta, uint32_t blockNumber,const char* partitionName);
void setBlockFree(FileSystemMeta* meta, uint32_t blockNumber,const char* partitionName);
uint32_t allocateBlock(FileSystemMeta* meta, const char* partitionName) ;
int saveBitmap(const FileSystemMeta* meta, const char* partitionName);
#endif // BITMAP_H

