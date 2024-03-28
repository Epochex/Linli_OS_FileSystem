//filesystem_api.h
#ifndef FILESYSTEM_API_H
#define FILESYSTEM_API_H

#include "filesystem_structs.h"

// 文件系统操作函数
int myFormat(const char* partitionName);
file* myOpen(const char* partitionName, const char* fileName);
int myWrite(file* f,  void* buffer, int nBytes, const char* partitionName );
int myRead(file* f, void* buffer, int nBytes);
void mySeek(file* f, int offset, int base);
void initFileSystem(const char* partitionName);
void myClose(file* f);
void deleteEntry(DirectoryEntry* entry);
void cleanupFileSystem();

#endif // FILESYSTEM_API_H
