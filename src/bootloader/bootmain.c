#include <stdint.h>
#include "../include/io/io.h"
#include "../include/c/string.h"
#include "../include/elf/elf.h"
#include "../include/mem/mem.h"
#include "../include/c/stdio.h"

#define SECTOR_SIZE 512
#define PAGE_SIZE 4096

// Should be equal to the "seek" in MakeFile for kernel
#define KERNEL_SEEK 50
#define KERNEL_ELF_DISK_OFFSET (0x200 * KERNEL_SEEK) - 0x200;

void readSegment(uint8_t *, uint32_t, uint32_t);

void loadOS() {
  ELF32_header *elf = (ELF32_header *)0x10000;

  readSegment((uint8_t *)elf, PAGE_SIZE, 0);

  uint32_t elfMagic = *(uint32_t *)elf->ident;

  // Is this an ELF executable?
  if (elfMagic != ELF_MAGIC) return;

  ELF32_programHeader *programHeader = (ELF32_programHeader *)((uint8_t *)elf + elf->programHeaderOffset);
  ELF32_programHeader *endProgramHeader = programHeader + elf->programHeaderNum;
  uint8_t *physicalAddress;

  for (; programHeader < endProgramHeader; programHeader++) {
    physicalAddress = (uint8_t *)programHeader->physicalAddress;
    readSegment(physicalAddress, programHeader->fileSize, programHeader->offset);

    // Fill remaining bytes with 0 in case the memory size of this section
    // is greater than the size in the ELF file
    if (programHeader->memorySize > programHeader->fileSize) {
      stosb(physicalAddress + programHeader->fileSize, 0, programHeader->memorySize - programHeader->fileSize);
    }
  }

  // Calculate the end of the kernel
  programHeader--;
  uint32_t kernelEnd = (uint32_t)(physicalAddress + programHeader->memorySize);

  initVideo();
  setForegroundColor(WHITE);
  setBackgroundColor(GREEN);
  printf("\nKernel end: %lx", kernelEnd);

  initializePhysicalMemoryManager(kernelEnd);
  initializeVirtualMemoryManager();

  // Call the entry point of the kernel (src/kernel/main.c -> OSStart)
  void (*entry)(void) = (void(*)(void))(elf->entry);
  entry();
}

/*
 * Since we haven't enable interrupts, we need to wait until the disk answers
 */
void waitDisk(void) {
  // Wait for disk ready.
  while((inb(DISK_PORT_BASE + 7) & 0xC0) != 0x40); // TODO: explain https://youtu.be/fZY1zr_nW6c?list=PLiUHDN3DAZZX_uTTp0l8QppxK3giZM2bC
}

/*
 * Read from the specified 'sector' and put in 'destination'
 */
void readSector(uint8_t *destination, uint32_t sector) {
  waitDisk();
  
  // Issue command
  outb(DISK_PORT_BASE + 2, 1); // count = 1 sector 
  outb(DISK_PORT_BASE + 3, sector); // Take the 1st LSB
  outb(DISK_PORT_BASE + 4, sector >> 8); // Take the 2nd LSB
  outb(DISK_PORT_BASE + 5, sector >> 16); // Take the 3rd LSB
  outb(DISK_PORT_BASE + 6, (sector >> 24) | 0xE0); // Take the 4th LSB (the MSB now), TODO: investigate the 0xE0
  outb(DISK_PORT_BASE + 7, DISK_READ_CMD); // cmd 0x20 - read sectors

  // Read data
  waitDisk();
  insl(DISK_PORT_BASE, destination, SECTOR_SIZE / 4); // Read 4-bytes 128 times, getting the 512 bytes
}

/*
 * Read 'bytes' from disk at 'address' and put them in 'destination' (physical address)
 */
void readSegment(uint8_t *destination, uint32_t bytes, uint32_t address) {
  // Take the kenel ELF offset in disk into consideration (stored in sector 4)
  address += KERNEL_ELF_DISK_OFFSET;

  // Calculate the final address
  uint8_t *endDestination = destination + bytes;

  // Calculate the sector to read
  // kernel starts at sector 4 (address 0x600 - 1536 bytes)
  // + 1 because LBAs start at 1
  int sector = (address / SECTOR_SIZE) + 1; 

  // This is needed becauses we read from disk in sectors
  // and if we want data at an address that is not SECTOR_SIZE align 
  // (a.k.a address % SECTOR_SIZE != 0) we will have data we did not requested
  // before the address we indicated.
  // for example we request 10 bytes from disk address 0xC8 (200 bytes),
  // this will be sector = (0xC8 / 512) + 1 = sector 1
  // and we get actually 512 bytes (address 0x0 to 0x200)
  // so, without the adjustment, 'destination' will point to address 0x0 instead of 0xC8 we wanted
  // with the adjustment we subtract (0xC8 % 512 = 0xC8), so the LOCAL 'destination' var
  // will point to 0x0 but the 'destination' in the caller will point to 0xC8 as we wanted
  destination -= address % SECTOR_SIZE;
  
  // Read sectors until filling the requested bytes amount
  for (; destination < endDestination; destination += SECTOR_SIZE, sector++)
    readSector(destination, sector);
}
