#pragma once
#include "kernel/fileSystem/fs.h"

class VirtualFileSystem {
  public:
    VirtualFileSystem();
    static VirtualFileSystem& instance();

    void loadSuperBlock();
    static void readSector(uint8_t *destination, uint32_t sector);

    int test;
    struct superBlock _superBlock;

  private:
    static void waitDisk(void);


    uint8_t tempSector[SECTOR_SIZE];
    uint8_t tempBlock[BLOCK_SIZE];
};
