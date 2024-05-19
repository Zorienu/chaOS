#include "VirtualFileSystem.h"
#include <kernel/utils/kprintf.h>
#include <io.h>
#include <string.h>

static VirtualFileSystem *_instance;

VirtualFileSystem::VirtualFileSystem() {
  _instance = this;
  this->test = 42;
}
 
VirtualFileSystem& VirtualFileSystem::instance() {
  return *_instance;
}

void VirtualFileSystem::loadSuperBlock() {
  VirtualFileSystem::readSector(_instance->tempSector, 1 * 8);
  memcpy(&_superBlock, tempSector, sizeof(struct superBlock));
}

/*
 * Since we haven't enable interrupts, we need to wait until the disk answers
 */
void VirtualFileSystem::waitDisk(void) {
  // Wait for disk ready.
  while((IO::inb(DISK_PORT_BASE + 7) & 0xC0) != 0x40); // TODO: explain https://youtu.be/fZY1zr_nW6c?list=PLiUHDN3DAZZX_uTTp0l8QppxK3giZM2bC
}

/*
 * Read from the specified 'sector' and put in 'destination'
 */
void VirtualFileSystem::readSector(uint8_t *destination, uint32_t sector) {
  waitDisk();
  
  // Issue command
  IO::outb(DISK_PORT_BASE + 2, 1); // count = 1 sector 
  IO::outb(DISK_PORT_BASE + 3, sector); // Take the 1st LSB
  IO::outb(DISK_PORT_BASE + 4, sector >> 8); // Take the 2nd LSB
  IO::outb(DISK_PORT_BASE + 5, sector >> 16); // Take the 3rd LSB
  IO::outb(DISK_PORT_BASE + 6, (sector >> 24) | 0xE0); // Take the 4th LSB (the MSB now)
                                                       // 0xE0 -> 0b1110_0000
                                                       // bit 4: drive number (0 in our case)
                                                       // bit 5: always set
                                                       // bit 6: set for LBA
                                                       // bit 7: always set
  IO::outb(DISK_PORT_BASE + 7, DISK_READ_CMD); // cmd 0x20 - read sectors

  // Read data
  waitDisk();
  IO::insl(DISK_PORT_BASE, destination, SECTOR_SIZE / 4); // Read 4-bytes 128 times, getting the 512 bytes
}
