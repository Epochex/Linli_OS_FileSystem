// main.c
#include <stdio.h>
#include "file_operations.h"

int main() {
    int choice;
    char partitionName[] = "partition.bin";
    char fileName[MAX_FILENAME_LENGTH];
    file* openFile = NULL;
    int bytesRead, bytesWritten;
    char buffer[1024]; // 用于读写的缓冲区
    int position, offset, base;

    // 测试 myFormat 函数
    printf("Formatting partition...\n");
    if (myFormat(partitionName) == 0) {
        printf("Partition formatted successfully.\n");
    } else {
        printf("Error formatting partition.\n");
        return 1; // 如果格式化失败，则退出程序
    }

    do {
        printf("\nFile System Menu:\n");
        printf("1. Open/Create File\n");
        printf("2. Write to File\n");
        printf("3. Read from File\n");
        printf("4. Seek File\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: // Open/Create File
                printf("Enter filename to open/create: ");
                scanf("%s", fileName);
                openFile = myOpen(fileName);
                if (openFile) {
                    printf("File '%s' opened/created successfully.\n", fileName);
                } else {
                    printf("Failed to open/create file '%s'.\n", fileName);
                }
                break;
            case 2: // Write to File
                if (openFile) {
                    printf("Enter string to write to file: ");
                    scanf("%s", buffer);
                    bytesWritten = myWrite(openFile, buffer, strlen(buffer));
                    if (bytesWritten >= 0) {
                        printf("Wrote %d bytes to file.\n", bytesWritten);
                    } else {
                        printf("Failed to write to file.\n");
                    }
                } else {
                    printf("No file is open. Please open a file first.\n");
                }
                break;
            case 3: // Read from File
                if (openFile) {
                    printf("Enter number of bytes to read: ");
                    scanf("%d", &bytesRead);
                    bytesRead = myRead(openFile, buffer, bytesRead);
                    if (bytesRead >= 0) {
                        buffer[bytesRead] = '\0'; // 确保字符串正确结束
                        printf("Read %d bytes: %s\n", bytesRead, buffer);
                    } else {
                        printf("Failed to read from file.\n");
                    }
                } else {
                    printf("No file is open. Please open a file first.\n");
                }
                break;
            case 4: // Seek File
                if (openFile) {
                    printf("Enter offset for seek: ");
                    scanf("%d", &offset);
                    printf("Enter seek base (0 for beginning, 1 for current, 2 for end): ");
                    scanf("%d", &base);
                    mySeek(openFile, offset, base);
                    printf("Seek operation completed.\n");
                } else {
                    printf("No file is open. Please open a file first.\n");
                }
                break;
            case 5: // Exit
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 5);

    return 0;
}
