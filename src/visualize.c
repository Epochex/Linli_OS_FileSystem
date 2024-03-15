// visualize.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "file_operations.h" // 假设这里定义了MAX_BLOCKS等

void visualizePartition() {
    int fd = open("partition.bin", O_RDONLY);
    if (fd == -1) {
        printf("Failed to open the partition file for visualization.\n");
        return;
    }

    int blockStatus[MAX_BLOCKS];
    read(fd, blockStatus, sizeof(blockStatus));

    printf("Partition Visualization:\n");
    for (int i = 0; i < MAX_BLOCKS; i++) {
        printf("%c", blockStatus[i] == 0 ? '_' : 'X');
    }
    printf("\n");

    close(fd);
}
