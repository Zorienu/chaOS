#pragma once
#include <stdint.h>

#define BLOCK_SIZE 4096

#define ROOT_DIRECTORY_INODE 1

enum FileType : uint8_t {
  FILETYPE_FILE = 0,
  FILETYPE_DIRECTORY = 1,
};

struct superBlock {
  // Inode bitmap
  uint32_t totalNumberOfInodes;
  uint32_t firstInodesBitmapBlock;  // What is the first block containing the bitmap for INodes?
  uint32_t inodesBitmapBlocks; // How many blocks the inode bitmap take?

  // Data blocks bitmap
  uint32_t totalNumberOfDataBlocks; // How many disk blocks are in the image? (including bootsector, superblock and bitmaps)
  uint32_t firstDataBlocksBitmapBlock; // What is the first block containing the bitmap for data blocks?
  uint32_t dataBlocksBitmapBlocks; // Hoy many blocks the data bitmap take?

  // Inode table
  uint32_t firstInodeBlock; // What is the first block containing the Inodes?
  uint32_t inodesBlocks; // How many blocks the inode table take?

  // Data blocks (where the actual data is)
  uint32_t firstDataBlock; // What is the first block containing the data blocks?

  uint16_t blockSizeInBytes; // What is the size of each block? Should be BLOCK_SIZE
  uint16_t inodeSizeInBytes; // What is the size of each INode? Should be sizeof(struct inode)
  uint32_t maxFileSize;
};

struct inode {
  uint32_t number; // INode number (The low level name of a file)
  uint32_t referenceCounter; // Reference counter
  uint32_t directDataBlocks[12]; // Direct data blocks (12 * 4096 = 49152 bytes)
  enum FileType type; // Wheter is a file or a directory (by now)
  uint32_t sizeInBytes; 
  uint32_t sizeInSectors;
};

struct directoryEntry {
  char name[60];
  uint32_t inodeNumber; 
};
