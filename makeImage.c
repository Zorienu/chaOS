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

uint32_t numberOfFiles = sizeof(files) / sizeof(struct file);

// We'll have numberOfFiles + 2 inodes
// inode 0 will be invalid
// inode 1 will be the root directory
uint32_t numberOfInodes;

static struct superBlock superBlock;

int min(int a, int b) {
  return a < b ? a : b;
}

int ceilDiv(int a, int b) {
  int result = a / b;

  if ((a % b) != 0) result++;

  return result;
}

int bytesToBlocks(int bytes) {
  return ceilDiv(bytes, BLOCK_SIZE);
}

int bytesToSectors(int bytes) {
  return ceilDiv(bytes, SECTOR_SIZE);
}

int sectorsToBytes(int sectors) {
  return sectors * SECTOR_SIZE;
}


void openFiles() {
  for (int i = 0; i < numberOfFiles; i++) 
    files[i].fd = open(files[i].name, O_RDONLY);

  outputImage.fd = open(outputImage.name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
}

void calculateFilesSize() {
  for (int i = 0; i < numberOfFiles; i++) {
    files[i].size = lseek(files[i].fd, 0, SEEK_END);
    lseek(files[i].fd, 0, SEEK_SET);
  }
}

void closeFiles() {
  for (int i = 0; i < numberOfFiles; i++) 
    close(files[i].fd);

  close(outputImage.fd);
}

void fillRemainingBytes(int remainingBytes) {
  if (remainingBytes == 0) return;

  uint8_t zero[SECTOR_SIZE];
  memset(zero, 0xFF, SECTOR_SIZE);

  while(remainingBytes > 0) {
    write(outputImage.fd, zero, min(SECTOR_SIZE, remainingBytes));
    remainingBytes -= SECTOR_SIZE;
  }
}

void writeSuperBlock() {
  superBlock.blockSizeInBytes = BLOCK_SIZE;
  superBlock.maxFileSize = 0xFFFFFFFF;

  superBlock.inodeSizeInBytes = sizeof(struct inode);
  superBlock.totalNumberOfInodes = numberOfInodes;
  superBlock.firstInodesBitmapBlock = 2; // 0 -> bootsector, 1 -> superblock
  superBlock.inodesBitmapBlocks = ceilDiv(numberOfInodes, BLOCK_SIZE * 8); // * 8: each byte holds 8 inodes

  superBlock.totalNumberOfDataBlocks = TOTAL_NUMBER_OF_DATA_BLOCKS;
  superBlock.firstDataBlocksBitmapBlock = superBlock.firstInodesBitmapBlock + superBlock.inodesBitmapBlocks;
  superBlock.dataBlocksBitmapBlocks = ceilDiv(superBlock.totalNumberOfDataBlocks, BLOCK_SIZE * 8); // * 8: each byte holds 8 inodes

  superBlock.firstInodeBlock = superBlock.firstDataBlocksBitmapBlock + superBlock.dataBlocksBitmapBlocks;
  superBlock.inodesBlocks = ceilDiv(numberOfInodes * sizeof(struct inode), BLOCK_SIZE);

  superBlock.firstDataBlock = superBlock.firstInodeBlock + superBlock.inodesBlocks;

  write(outputImage.fd, &superBlock, sizeof(struct superBlock));
  int remainingBytes = BLOCK_SIZE - (sizeof(superBlock) % BLOCK_SIZE);
  fillRemainingBytes(remainingBytes);
}

void writeInodesBitmap() {
  int inodesBitmapBlocks = ceilDiv(numberOfInodes, BLOCK_SIZE * 8); // * 8: each byte holds 8 inodes
  int inodesBitmapBytes = inodesBitmapBlocks * BLOCK_SIZE;
  uint8_t *inodesBitmap = (uint8_t *)malloc(inodesBitmapBytes);
  memset(inodesBitmap, 0x0, inodesBitmapBytes);

  int byte = 0;
  for (int i = 0; i < numberOfInodes; i++) {
    inodesBitmap[byte] |= 1 << i;

    if (i == 0) continue;
    byte += i % 8 == 0 ? 1 : 0;
  }

  write(outputImage.fd, inodesBitmap, inodesBitmapBytes);
}

void writeDataBlocksBitmap() {
  int dataBlocksBitmapBlocks = ceilDiv(TOTAL_NUMBER_OF_DATA_BLOCKS, BLOCK_SIZE * 8); // * 8: each byte holds 8 inodes
  int dataBlocksBitmapBytes = dataBlocksBitmapBlocks * BLOCK_SIZE;
  uint8_t *dataBlocksBitmap= (uint8_t *)malloc(dataBlocksBitmapBytes);
  memset(dataBlocksBitmap, 0x0, dataBlocksBitmapBytes);

  // Data block 0 -> bootsector
  // Data block 1 -> superblock
  // Data block 2 -> inodes bitmap
  // Data block 3 -> data blocks bitmap
  // Data block 4 -> inodes (see superblock to get the amount of blocks for inodes)
  int reservedDataBlocks = superBlock.firstDataBlock;
  int rootDirectoryBlocks = ceilDiv(numberOfFiles * sizeof(struct directoryEntry), BLOCK_SIZE);

  int filesBlocks = 0;
  // Starting from 1 for ignoring bootsector (already taken into account)
  for (int i = 1; i < numberOfFiles; i++) {
    filesBlocks += bytesToBlocks(files[i].size);
  }

  int totalUsedDataBlocks = reservedDataBlocks + rootDirectoryBlocks + filesBlocks;
  printf("Reserved data blocks: %d\n", reservedDataBlocks);
  printf("Root directory data blocks: %d\n", rootDirectoryBlocks);
  printf("Files data blocks: %d\n", filesBlocks);
  printf("Total data blocks: %d\n", totalUsedDataBlocks);

  int byte = 0;
  for (int i = 0; i < totalUsedDataBlocks; i++) {
    if (i != 0) byte += i % 8 == 0 ? 1 : 0;
     
    dataBlocksBitmap[byte] |= 1 << (i % 8);
  }

  write(outputImage.fd, dataBlocksBitmap, dataBlocksBitmapBytes);
}

void writeInodes() {
  // We'll have numFiles + 2 (the root directory) inodes
  // inode 0 will be invalid
  // inode 1 will be the root directory
  struct inode inodes[numberOfFiles + 2];
  memset(inodes, 0x0, sizeof(inodes));

  // First we need to create the root directory
  inodes[1] = (struct inode) {
    .number = 1,
    .referenceCounter = 1,
    .directDataBlocks = { 0 },
    .type = FILETYPE_DIRECTORY,
    // By now we have 3 files (bootsector.bin, prekernel.bin and kernel.bin)
    .sizeInBytes = (uint32_t)(numberOfFiles * sizeof(struct directoryEntry)), 
    .sizeInSectors = (uint32_t)ceilDiv(numberOfFiles * sizeof(struct directoryEntry), SECTOR_SIZE),
  };

  // Point inode to the data blocks
  int nextDataBlock = superBlock.firstDataBlock;
  for (int i = 0; i < bytesToBlocks(inodes[1].sizeInBytes); nextDataBlock++, i++) {
    inodes[1].directDataBlocks[i] = nextDataBlock;
  }

  // inode 0 -> invalid
  // inode 1 -> root directory
  uint32_t nextInodeNumber = 2;

  for (int i = 0; i < numberOfFiles; i++) {
    inodes[nextInodeNumber] = (struct inode) {
      .number = nextInodeNumber,
      .referenceCounter = 1,
      .directDataBlocks = { 0 },
      .type = FILETYPE_FILE,
      .sizeInBytes = files[i].size,
      .sizeInSectors = (uint32_t)ceilDiv(files[i].size, SECTOR_SIZE),
    };

    // Set the datablocks used for each file
    // Ignore bootsector file since it is in block 0
    if (i != 0) 
      for (int j = 0; j < bytesToBlocks(inodes[nextInodeNumber].sizeInBytes); nextDataBlock++, j++) {
        inodes[nextInodeNumber].directDataBlocks[j] = nextDataBlock;
      }

    nextInodeNumber++;
  }

  write(outputImage.fd, inodes, sizeof(inodes));

  // Fill remaining block with zeros
  int remainingBytes = BLOCK_SIZE - (sizeof(inodes) % BLOCK_SIZE);
  fillRemainingBytes(remainingBytes);
}

void writeBinaryToImg(struct file file) {
  int nread = 0;
  char buffer[SECTOR_SIZE];

  while ((nread = read(file.fd, buffer, sizeof(buffer))) > 0) {
    int nwrite = write(outputImage.fd, buffer, sizeof(buffer));

    // Clear buffer
    memset(buffer, 0x0, sizeof(buffer));

    if (nwrite < nread) {
      printf("Error writing buffer to image, error: %s", strerror(errno));
    }
  } 

  // We use sectors instead of "size" because we may ended up writing more than "size" (we write by sectors)
  int writtenSectors = bytesToSectors(file.size);
  int remainingBytes = BLOCK_SIZE - (sectorsToBytes(writtenSectors) % BLOCK_SIZE);
  printf("Filling remaining bytes %d for file %s\n", remainingBytes, file.name);
  fillRemainingBytes(remainingBytes);
}
}

void printCurrentImgPosition() {
  printf("Current position on file: %llx\n", lseek(outputImage.fd, 0x0, SEEK_CUR));
}

int main() {
  printf("Creating chaOS image\n");

  numberOfInodes = numberOfFiles + 2;

  openFiles();

  if (outputImage.fd < 0) { printf("Error creating image.img, error: %s\n", strerror(errno)); return 1; }

  printf("Bootsector FD: %d\n", files[BINARY_FILE_BOOTSECTOR].fd);
  printf("Prekernel FD: %d\n", files[BINARY_FILE_PREKERNEL].fd);
  printf("Kernel FD: %d\n", files[BINARY_FILE_KERNEL].fd);
  printf("Image FD: %d\n", outputImage.fd);

  calculateFilesSize();

  printf("\nBinaries size:\n");
  printf("Bootsector size in bytes: %d, blocks: %d, sectors: %d\n", files[BINARY_FILE_BOOTSECTOR].size, bytesToBlocks(files[BINARY_FILE_BOOTSECTOR].size), ceilDiv(files[BINARY_FILE_BOOTSECTOR].size, SECTOR_SIZE));
  printf("Prekernel size in bytes: %d, blocks: %d, sectors: %d\n", files[BINARY_FILE_PREKERNEL].size, bytesToBlocks(files[BINARY_FILE_PREKERNEL].size), ceilDiv(files[BINARY_FILE_PREKERNEL].size, SECTOR_SIZE));
  printf("Kernel size in bytes: %d, blocks: %d, sectors: %d\n", files[BINARY_FILE_KERNEL].size, bytesToBlocks(files[BINARY_FILE_KERNEL].size), ceilDiv(files[BINARY_FILE_KERNEL].size, SECTOR_SIZE));

  printf("Writing bootsector to img...\n");
  writeBinaryToImg(BINARY_FILE_BOOTSECTOR); 
  printCurrentImgPosition();

  printf("Writing superblock to img...\n");
  writeSuperBlock();
  printCurrentImgPosition();
  
  printf("Writing inodes bitmap to img...\n");
  writeInodesBitmap();
  printCurrentImgPosition();

  printf("Writing datablocks bitmap to img...\n");
  writeDataBlocksBitmap();
  printCurrentImgPosition();

  printf("Writing inodes to img...\n");
  writeInodes();
  printCurrentImgPosition();

  printf("Writing data blocks to img...\n");
  writeDataBlocks();

  closeFiles();

  printf("Size of INode: %lu bytes\n", sizeof(struct inode));
  printf("Size of Superblock: %lu bytes\n", sizeof(struct superBlock));
  printf("Size of DirectoryEntry: %lu bytes\n", sizeof(struct directoryEntry));
  printf("Finished creating img...\n");

  writeInodes();

  return 0;
}
