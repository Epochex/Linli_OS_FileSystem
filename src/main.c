#include "filesystem_structs.h"
#include "filesystem_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printMenu() {
    printf("\nFile System Simulator\n");
    printf("=====================\n");
    printf("1. Format FileSystem\n");
    printf("2. Create/Open File\n");
    printf("3. Write to File\n");
    printf("4. Read from File\n");
    printf("5. Find File/Directory\n");
    printf("6. Print File Tree\n");
    printf("7. Exit\n");    
    printf("Select an option: ");
}

void pauseAndClearScreen() {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n'); // Wait for Enter key to be pressed
    getchar(); // Consume the Enter key

    // Use 'cls' on Windows, 'clear' on Unix/Linux
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

int main() {
    const char* partitionName = "filesystem.img";
    file* myFile = NULL;
    char fileName[256];
    char inputBuffer[1024];
    char data[1024];
    int option = 0;
    
    initFileSystem(partitionName);

    while (1) {
        printMenu();
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) continue;
        if (sscanf(inputBuffer, "%d", &option) != 1) {
            printf("Invalid input, please try again.\n");
            pauseAndClearScreen();
            continue;
        }
        
        switch (option) {
            case 1:
                if (myFormat(partitionName) != 0) {
                    fprintf(stderr, "Failed to format partition.\n");
                } else {
                    printf("Partition formatted successfully.\n");
                }
                break;
            case 2:
                printf("Enter file name: ");
                if (fgets(fileName, sizeof(fileName), stdin) == NULL) {
                    printf("Error reading file name.\n");
                    continue;
                }
                // Remove the newline at the end of the input
                fileName[strcspn(fileName, "\n")] = 0;

                myFile = myOpen(partitionName, fileName);
                if (myFile == NULL) {
                    fprintf(stderr, "Failed to open file: %s\n", fileName);
                } else {
                    printf("File '%s' opened successfully.\n", fileName);
                }
                break;
            case 3:
                if (myFile == NULL) {
                    printf("No file is open.\n");
                    break;
                }
                printf("Enter data to write: ");
                if (fgets(data, sizeof(data), stdin) == NULL) {
                    printf("Error reading input data.\n");
                    continue;
                }
                // Remove the newline at the end of the input
                data[strcspn(data, "\n")] = 0;

                int dataSize = strlen(data) + 1; // +1 for null terminator
                if (myWrite(myFile, (void*)data, dataSize, partitionName) < dataSize) {
                    fprintf(stderr, "Failed to write data to file.\n");
                } else {
                    printf("Data written successfully.\n");
                }
                break;
            case 4:
                if (myFile == NULL) {
                    printf("No file is open.\n");
                    break;
                }
                char buffer[1024]; // Define the buffer array
                mySeek(myFile, 0, SEEK_SET); // Move to the beginning of the file

                int bytesRead = myRead(myFile, buffer, sizeof(buffer) - 1);
                if (bytesRead < 0) {
                    fprintf(stderr, "Failed to read data from file.\n");
                } else {
                    buffer[bytesRead] = '\0'; // Null-terminate the string
                    printf("Read from file: %s\n", buffer);
                }
                break;

                pauseAndClearScreen();
                break;
        
            case 5:
                printf("Enter the path of the file/directory to find: ");
                if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) {
                    printf("Error reading path.\n");
                    continue;
                }
                // Remove the newline at the end of the input
                inputBuffer[strcspn(inputBuffer, "\n")] = 0;

                DirectoryEntry* foundEntry = findEntry(fs.root, inputBuffer);
                if (foundEntry == NULL) {
                    printf("File/Directory not found.\n");
                } else {
                    printf("File/Directory '%s' found.\n", foundEntry->entryName);
                    // 如果是文件，可以显示更多信息
                    if (foundEntry->fileMeta != NULL) {
                        printf("File size: %u bytes\n", foundEntry->fileMeta->fileSize);
                    }
                }
                break;

            case 6:
                printf("Directory Tree:\n");
                printDirectoryTree(fs.root, 0);  // 从根目录开始，深度为0
                break;

            case 7:
                printf("Exiting...\n");
                if (myFile) {
                    myClose(myFile); // Close the file if it's open
                }
                cleanupFileSystem(); // Perform any necessary cleanup
                return 0;

            default:
                printf("Invalid option, please try again.\n");
                pauseAndClearScreen();
            
        }
    }
    return 0;
}





