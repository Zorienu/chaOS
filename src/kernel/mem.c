#include <stddef.h>
#include <stdint.h>
#include "stdio.h"
#include "memLayout.h"
#include "mmu.h"

#define SMAP_NUM_ENTRIES_ADDRESS 0xA500;
#define SMAP_ENTRIES_ADDRESS     0xA504;

extern uint32_t *kernelEnd;

typedef struct {
  uint64_t baseAddress; // Base address of the address range
  uint64_t length;      // Length of address range in bytes
  uint32_t type;        // Type of address range (enum )
  uint32_t acpi;
} SMAP_entry_t;

enum SMAP_entry_type {
  AVAILABLE           = 1, 
  reserved            = 2, // e.g. system ROM, memory-mapped devices
  ACPI_RECLAIM_MEMORY = 3, // Usable by OS after reading ACPI tables
  ACPI_NVS_MEMORY     = 4 // OS is required to save this memory between NVS sessions
};

void memcpy(void *_destination, void *_source, size_t count) {
  uint8_t *destination = _destination;
  uint8_t *source = _source;

  while (count--) 
    *destination++ = *source++;
}
 
void memset(void *_destination, uint8_t value, size_t count) {
  uint8_t *destination = _destination;

  while (count--)
    *destination++ = value;
}

void kmalloc(size_t count) {
  int kernelEndPhys = (uint32_t)V2P(&kernelEnd);
  int alignedAddress = kernelEndPhys + PAGE_SIZE - (kernelEndPhys % PAGE_SIZE);

  puts("\nKernel physical memory end: ");
  putNumber(kernelEndPhys, 16);
  puts("\nKernel physical memory end aligned: ");
  putNumber(alignedAddress, 16);

  int pages = 0;

  for (; alignedAddress < PHYSICAL_STOP; alignedAddress += PAGE_SIZE){
    pages++;
  }

    puts("\nPages: ");
    putNumber(pages, 10);
    puts("\nFinal page address: ");
    putNumber(alignedAddress, 16);

}
