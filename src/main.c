// main.c
#include <stdio.h>
#include "file_operations.h"

int main() {
    // 测试 myFormat 函数
    if (myFormat("partition.bin") == 0) {
        printf("Partition formatted successfully.\n");
    } else {
        printf("Error formatting partition.\n");
    }

    // ...其他代码...
    return 0;
}
