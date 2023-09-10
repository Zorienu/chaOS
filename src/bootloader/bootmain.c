#include "../kernel/elf.h"
#include "../kernel/io.h"
#include <stdint.h>

#define SECTOR_SIZE 512
#define PAGE_SIZE 4096

void readSegment(uint8_t *, uint32_t, uint32_t);

void loadOS() {
  ELF32_header *elf = (ELF32_header *)0x10000;

  readSegment((uint8_t *)elf, PAGE_SIZE / 4, -1);
  uint16_t *vga = (uint16_t *)0xB8000;
  *vga = 1 << 12 | 5 << 8 | '@';
}

/*
 * Since we haven't enable interrupts, we need to wait until the disk answers
 */
void waitDisk(void) {
  // Wait for disk ready.
  while((inb(0x1F7) & 0xC0) != 0x40);
}

void readSector(uint8_t *destination, uint32_t sector) {

  waitDisk();
  
  // Issue command
  outb(0x1F2, 1); // count = 1 sector 
  outb(0x1F3, sector); // Take the 1st LSB
  outb(0x1F4, sector >> 8); // Take the 2nd LSB
  outb(0x1F5, sector >> 16); // Take the 3rd LSB
  outb(0x1F6, (sector >> 24) | 0xE0); // Take the 4th LSB (the MSB now), TODO: investigate the 0xE0
  outb(0x1F7, 0x20); // cmd 0x20 - read sectors

  // Read data
  waitDisk();
  insl(0x1F0, destination, SECTOR_SIZE / 4); // Read 4-bytes 128 times, getting the 512 bytes
}

/*
 * Read 'bytes' from disk at 'address' and put them in 'destination' (physical address)
 */
void readSegment(uint8_t *destination, uint32_t bytes, uint32_t address) {
  // Calculate the final address
  uint8_t *endDestination = destination + bytes;

  // Calculate the sector to read
  // kernel starts at sector 1 (address 0x200 - 512 bytes)
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
