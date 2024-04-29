#include <stdint.h>
#include <io.h>
#include <string.h>
#include <elf.h>
#include <mem.h>
#include <stdio.h>
#include <kernel/fileSystem/fs.h>
#include <kernel/fileSystem/VirtualFileSystem.h>

#define SECTOR_SIZE 512
#define PAGE_SIZE 4096

// Should be equal to the "seek" in MakeFile for kernel
#define KERNEL_SEEK 100
#define KERNEL_ELF_DISK_OFFSET 0xD000;

static uint32_t kernelELFDiskOffset;

void readSegment(uint8_t *, uint32_t, uint32_t);

__attribute__ ((section ("prekernel_entry"))) void loadOS() {
  // We will put the elf header in this address
  ELF32_header *elf = (ELF32_header *)0x10000;

  initVideo();
  setForegroundColor(WHITE);
  setBackgroundColor(BLUE);
  // TODO: use inodes to locate kernel binary
  printf("\n\nKernel ELF header address: %lx\n", elf);

  uint8_t tmp[SECTOR_SIZE];
  struct superBlock superblock;// = (struct superBlock*)tmp;

  // Read the first sector of the super block, right after the boot block
  VirtualFileSystem::readSector(tmp, 1 * 8);
  memcpy(&superblock, tmp, sizeof(struct superBlock));

  printf("SuperBlock Info:\n");
  printf("SuperBlock address: %lx\n", superblock);
  printf("totalNumberOfInodes: %d\n", superblock.totalNumberOfInodes);
  printf("firstInodeBlock: %d\n", superblock.firstInodeBlock);
  printf("inodesBlocks: %d\n", superblock.inodesBlocks);
  printf("firstDataBlock: %d\n", superblock.firstDataBlock);

  struct inode inodes[4];

  VirtualFileSystem::readSector(tmp, superblock.firstInodeBlock * 8);
  memcpy(&inodes, tmp, sizeof(struct inode) * 4);

  struct inode rootDirectoryInode = inodes[1];

  printf("Root directory inode info: \n");
  printf("number: %d\n", rootDirectoryInode.number);
  printf("first direct dataBlock: %d\n", rootDirectoryInode.directDataBlocks[0]);
  printf("type: %s\n", rootDirectoryInode.type ? "directory" : "file");
  printf("sizeInBytes: %d\n", rootDirectoryInode.sizeInBytes);
  printf("sizeInSectors: %d\n", rootDirectoryInode.sizeInSectors);

  struct directoryEntry rootDirectoryEntries[4];

  VirtualFileSystem::readSector(tmp, rootDirectoryInode.directDataBlocks[0] * 8);
  memcpy(&rootDirectoryEntries, tmp, sizeof(rootDirectoryEntries));

  printf("\nRoot directory content: \n");
  for (int i = 0; i < 3; i++) {
    printf("Name: %s, inode: %d\n", rootDirectoryEntries[i].name, rootDirectoryEntries[i].inodeNumber);
  }

  struct directoryEntry *kernelDirectoryEntry = &rootDirectoryEntries[0];
  while (!strcmp(kernelDirectoryEntry->name, "kernel")) kernelDirectoryEntry++;


  printf("\nKernel\n");
  printf("Name: %s, inode: %d\n", kernelDirectoryEntry->name, kernelDirectoryEntry->inodeNumber);

  // Load the sector containing the kernel inode (struct)
  int inodesPerSector = SECTOR_SIZE / sizeof(struct inode); // 4 
  // 4 is the first inode block, 8 is the number of sectors by block
  int sectorToLoad = kernelDirectoryEntry->inodeNumber / inodesPerSector + (4 * 8);

  printf("Kernel inode sector to Load: %d\n", sectorToLoad);


  struct inode kernelInode;
  VirtualFileSystem::readSector(tmp, sectorToLoad);
  memcpy(&kernelInode, tmp, sizeof(struct inode));

  printf("\nKernel inode\n");
  printf("number: %d\n", kernelInode.number);
  printf("first direct dataBlock: %d\n", kernelInode.directDataBlocks[0]);
  printf("type: %s\n", kernelInode.type ? "directory" : "file");
  printf("sizeInBytes: %d\n", kernelInode.sizeInBytes);
  printf("sizeInSectors: %d\n", kernelInode.sizeInSectors);

  kernelELFDiskOffset = kernelInode.directDataBlocks[0] * PAGE_SIZE;

  // Load 4096 bytes from hard disk address KERNEL_ELF_DISK_OFFSET into 0x10_000
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
      IO::stosb(physicalAddress + programHeader->fileSize, 0, programHeader->memorySize - programHeader->fileSize);
    }
  }


  // Calculate the end of the kernel
  programHeader--;
  uint32_t kernelEnd = (uintptr_t)(physicalAddress + programHeader->memorySize);


  initializePhysicalMemoryManager(kernelEnd);
  initializeVirtualMemoryManager();


  // Call the entry point of the kernel (src/kernel/main.c -> OSStart)
  void (*entry)(void) = (void(*)(void))(elf->entry);
  // while(1);
  entry();
}

/*
 * Read 'bytes' from disk at 'address' and put them in 'destination' (physical address)
 */
void readSegment(uint8_t *destination, uint32_t bytes, uint32_t address) {
  // Take the kenel ELF offset in disk into consideration (stored in sector 4)
  address += kernelELFDiskOffset;

  // Calculate the final address
  uint8_t *endDestination = destination + bytes;

  // Calculate the sector to read
  // kernel starts at sector 4 (address 0x600 - 1536 bytes)
  // + 1 because LBAs start at 1
  int sector = address / SECTOR_SIZE; 

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
    VirtualFileSystem::readSector(destination, sector);
}
