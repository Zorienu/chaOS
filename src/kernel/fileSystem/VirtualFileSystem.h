#pragma once
#include "kernel/fileSystem/fs.h"

class VirtualFileSystem {
  public:
    void loadSuperBlock();
    static void readSector(uint8_t *destination, uint32_t sector);

  private:
    static void waitDisk(void);

    struct superBlock _superBlock;
};
