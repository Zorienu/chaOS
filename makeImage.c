#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include "./src/kernel/fileSystem/fs.h"

#define SECTOR_SIZE 512
#define TOTAL_IMAGE_SIZE 1.44 * 1000 * 1000 // Lets try with a floppy disk size
#define TOTAL_NUMBER_OF_DATA_BLOCKS TOTAL_IMAGE_SIZE / BLOCK_SIZE
// #define PRINT_HEX

enum BinaryFiles : uint8_t {
  BINARY_FILE_BOOTSECTOR = 0,
  BINARY_FILE_PREKERNEL = 1,
  BINARY_FILE_KERNEL = 2,
};

struct file {
  char name[60];
  uint8_t fd;
  uint32_t size;
};

struct file outputImage = { "build/bin/image.img", 0, 0 };

struct file files[] = {
  { "build/bin/bootsector", 0, 0 },
  { "build/bin/prekernel", 0, 0 },
  { "build/bin/kernel", 0, 0 },
};

uint32_t numFiles = sizeof(files) / sizeof(struct file);

int min(int a, int b) {
  return a < b ? a : b;
}

void openFiles() {
  for (int i = 0; i < numFiles; i++) 
    files[i].fd = open(files[i].name, O_RDONLY);

  outputImage.fd = open(outputImage.name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
}

void calculateFilesSize() {
  for (int i = 0; i < numFiles; i++) {
    files[i].size = lseek(files[i].fd, 0, SEEK_END);
    lseek(files[i].fd, 0, SEEK_SET);
  }
}

void closeFiles() {
  for (int i = 0; i < numFiles; i++) 
    close(files[i].fd);

  close(outputImage.fd);
}

void fillRemainingBytes(int remainingBytes) {
  uint8_t zero[SECTOR_SIZE];
  memset(zero, 0xFF, SECTOR_SIZE);

  while(remainingBytes > 0) {
    write(outputImage.fd, zero, min(SECTOR_SIZE, remainingBytes));
    remainingBytes -= SECTOR_SIZE;
  }
}

void writeInodes() {
  // We'll have numFiles + 1 (the root directory) inodes
  struct inode inodes[numFiles + 1];
  memset(inodes, 0x0, sizeof(inodes));

  // First we need to create the root directory
  inodes[0] = (struct inode) {
    .number = 1,
    .referenceCounter = 1,
    .directDataBlocks = { 0 },
    .type = FILETYPE_DIRECTORY,
    // By now we have 3 files (bootsector.bin, prekernel.bin and kernel.bin)
    .sizeInBytes = (uint32_t)(numFiles * sizeof(struct directoryEntry)), 
    .sizeInSectors = 1,
  };

  uint32_t nexInodeNumber = 2;

  for (int i = 0; i < numFiles; i++) {
    inodes[i] = (struct inode) {
      .number = nexInodeNumber++,
      .referenceCounter = 1,
      .directDataBlocks = { 0 },
      .type = FILETYPE_FILE,
      .sizeInBytes = files[i].size,
      .sizeInSectors = files[i].size / SECTOR_SIZE,
    };
  }
}

void writeBinaryToImg(enum BinaryFiles file) {
  int nread = 0;
  char buffer[SECTOR_SIZE];

  while ((nread = read(files[file].fd, buffer, sizeof(buffer))) > 0) {
    int nwrite = write(outputImage.fd, buffer, sizeof(buffer));

    // Clear buffer
    memset(buffer, 0x0, sizeof(buffer));

    if (nwrite < nread) {
      printf("Error writing buffer to image, error: %s", strerror(errno));
    }
  } 
}

int main() {
  printf("Creating chaOS image\n");

  openFiles();

  if (outputImage.fd < 0) { printf("Error creating image.img, error: %s\n", strerror(errno)); return 1; }

  printf("Bootsector FD: %d\n", files[BINARY_FILE_BOOTSECTOR].fd);
  printf("Prekernel FD: %d\n", files[BINARY_FILE_PREKERNEL].fd);
  printf("Kernel FD: %d\n", files[BINARY_FILE_KERNEL].fd);
  printf("Image FD: %d\n", outputImage.fd);

  calculateFilesSize();

  printf("\nBinaries size:\n");
  printf("Bootsector size in bytes: %d\n", files[BINARY_FILE_BOOTSECTOR].size);
  printf("Prekernel size in bytes: %d\n", files[BINARY_FILE_PREKERNEL].size);
  printf("Kernel size in bytes: %d\n", files[BINARY_FILE_KERNEL].size);

  char buffer[512];
  memset(buffer, 0x0, sizeof(buffer));
  int nread = 0;

  printf("Writing bootsector to img...\n");
  writeBinaryToImg(BINARY_FILE_BOOTSECTOR);
  printf("Writing prekernel to img...\n");
  writeBinaryToImg(BINARY_FILE_PREKERNEL);

  lseek(outputImage.fd, 0x200 * 100, SEEK_SET);
  printf("Writing kernel to img...\n");
  writeBinaryToImg(BINARY_FILE_KERNEL);

  closeFiles();

  printf("Size of INode: %lu bytes\n", sizeof(struct inode));
  printf("Size of Superblock: %lu bytes\n", sizeof(struct superBlock));
  printf("Finished creating img...\n");

  writeInodes();

  return 0;
}
