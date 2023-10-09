#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "memLayout.h"
#include "virtualMem.h"
#include "../c/stdio.h"
#include "../c/string.h"

#define SMAP_NUM_ENTRIES_ADDRESS 0xA500;
#define SMAP_ENTRIES_ADDRESS     0xA504;

// extern uint32_t *kernelEnd;

typedef struct {
  uint64_t baseAddress; // Base address of the address range
  uint64_t length;      // Length of address range in bytes
  uint32_t type;        // Type of address range (enum )
  uint32_t acpi;
} SMAP_entry_t;

enum SMAP_entry_type {
  AVAILABLE           = 1, 
  RESERVED            = 2, // e.g. system ROM, memory-mapped devices
  ACPI_RECLAIM_MEMORY = 3, // Usable by OS after reading ACPI tables
  ACPI_NVS_MEMORY     = 4 // OS is required to save this memory between NVS sessions
};

#define MAX_BLOCKS_AMOUNT 1024 * 1024

struct {
  // TODO: add lock
  uint32_t availableMemoryStack[MAX_BLOCKS_AMOUNT];
  uint32_t *top;
} physicalMemory;



uint32_t availableBlocks = 0;

void freeBlock(uint32_t address) {
  // We cannot fill the block with junk because is not pointed by any PET
  // doing that will cause a page fault

  availableBlocks++;
  *physicalMemory.top-- = address;
}

uint32_t getBlock() {
  if (physicalMemory.top == physicalMemory.availableMemoryStack + MAX_BLOCKS_AMOUNT - 1)
    return 0;

  availableBlocks--;
  return *++physicalMemory.top;
}

void printMemoryMap() {
  uint32_t SMAPNumEntries = *(uint32_t *)SMAP_NUM_ENTRIES_ADDRESS;
  SMAP_entry_t *SMAPEntry = (SMAP_entry_t *)SMAP_ENTRIES_ADDRESS;

  uint32_t availableBlocks = 0;
  uint32_t reservedBlocks = 0;

  for (int i = 0; i < SMAPNumEntries; i++) {
    printf("\nRegion: %d", i);
    printf(" Base Address: %x", SMAPEntry->baseAddress);
    printf(" Length: %x", SMAPEntry->length);
    printf(" Type: %d", SMAPEntry->type);

    if (SMAPEntry->type == AVAILABLE) 
      availableBlocks += SMAPEntry->length;
    else 
      reservedBlocks += SMAPEntry->length;

    SMAPEntry++;
  }

  printf("\n\nTotal memory in bytes: %x", SMAPEntry->baseAddress + SMAPEntry->length - 1);
  printf("\nAvailable blocks: %x", availableBlocks / PAGE_SIZE);
  printf("\nReserved blocks: %x", reservedBlocks / PAGE_SIZE);
}

void initializePhysicalMemoryManager() {
  // Initialize stack
  physicalMemory.top = physicalMemory.availableMemoryStack + MAX_BLOCKS_AMOUNT - 1;

  uint32_t SMAPNumEntries = *(uint32_t *)SMAP_NUM_ENTRIES_ADDRESS;
  SMAP_entry_t *SMAPEntry = (SMAP_entry_t *)SMAP_ENTRIES_ADDRESS;

  for (int i = 0; i < SMAPNumEntries; i++) {
    if (SMAPEntry->type == AVAILABLE) {
      // Allocate only available memory
      uint32_t alignedBaseAddress = ALIGN_ADDRESS_UP(SMAPEntry->baseAddress);

      for (int offset = 0; offset + PAGE_SIZE < SMAPEntry->length; offset += PAGE_SIZE) 
        // if ((alignedBaseAddress + offset) > V2P(&kernelEnd))
          freeBlock(alignedBaseAddress + offset);
    }

    // Continue with the next entry in the table
    SMAPEntry++;
  }

  // printf("\n\n--Stack--");
  // printf("\nStack address: %x", physicalMemory.availableMemoryStack);
  // printf("\nTop address: %x", physicalMemory.top);
  // printf("\nKernel end: %x", &kernelEnd);
  // printf("\nKernel end: %x", V2P(&kernelEnd));
  // printf("\nAvailable blocks: %d", availableBlocks);
  // printf("\nfirst allocation: %x", getBlock());
  // printf("\nsecond allocation: %x", getBlock());
}

void map4MBPage(PageTable *pageTable, PhysicalAddress physicalFrame, VirtualAddress virtualAddress) {
  for (uint32_t i = 0; i < PAGES_PER_TABLE; i++, physicalFrame += PAGE_SIZE, virtualAddress += PAGE_SIZE) {
    // Create new page entry
    PageTableEntry entry = 0;

    setAttribute(&entry, PTE_PRESENT);
    setAttribute(&entry, PTE_READ_WRITE);
    setPhysicalFrame(&entry, physicalFrame);

    // Add page entry to the page table
    pageTable->entries[PT_INDEX(virtualAddress)] = entry;
  }
}

bool initializeVirtualMemoryManager() {
  PageDirectory *pageDirectory = (PageDirectory *)getBlock();

  // Out of memory
  if (!pageDirectory) return false;

  memset(pageDirectory, 0, sizeof(PageDirectory));

  for (int i = 0; i < TABLES_PER_DIRECTORY; i++) {
    pageDirectory->entries[i] = PTE_READ_WRITE; // Supervisor, read/write, not present
  }

  // For identity map from 0x0 to 4MB
  PageTable *identityPageTable = (PageTable *)getBlock();
  // Out of memory
  if (!identityPageTable) return false;
  memset(identityPageTable, 0, sizeof(PageTable));
  // Fill identity mapped page table from 0x0 to 4MB
  map4MBPage(identityPageTable, 0x0, 0x0);

  // For identity map from 4MB to 8MB
  PageTable *identityPageTable2 = (PageTable *)getBlock();
  // Out of memory
  if (!identityPageTable2) return false;
  memset(identityPageTable2, 0, sizeof(PageTable));
  // Fill identity mapped page table from 4MB to 8MB
  map4MBPage(identityPageTable2, 4 * MB, 4 * MB);


  // For mapping from KERNEL_BASE to 0x0 (0x0 to 4MB)
  PageTable *pageTable3GB = (PageTable *)getBlock();
  // Out of memory
  if (!pageTable3GB) return false;
  memset(pageTable3GB, 0, sizeof(PageTable));
  // Fill page table for mapping 3GB to 0x0 (where the kernel resides)
  map4MBPage(pageTable3GB, 0x0, KERNEL_BASE);

  // For mapping from KERNEL_BASE to 0x0 (0x0 to 4MB)
  PageTable *pageTable3GB2 = (PageTable *)getBlock();
  // Out of memory
  if (!pageTable3GB2) return false;
  memset(pageTable3GB2, 0, sizeof(PageTable));
  // Fill page table for mapping (3GB + 4MB) to 4MB (where the kernel resides)
  map4MBPage(pageTable3GB2, 4 * MB, KERNEL_BASE + 4 * MB);


  PageDirectoryEntry *entry1 = &pageDirectory->entries[PD_INDEX(0x0)];
  setAttribute(entry1, PTE_PRESENT);
  setAttribute(entry1, PTE_READ_WRITE);
  setPhysicalFrame(entry1, (PhysicalAddress)identityPageTable);

  PageDirectoryEntry *entry2 = &pageDirectory->entries[PD_INDEX(4 * MB)];
  setAttribute(entry2, PTE_PRESENT);
  setAttribute(entry2, PTE_READ_WRITE);
  setPhysicalFrame(entry2, (PhysicalAddress)identityPageTable2);



  PageDirectoryEntry *entry3 = &pageDirectory->entries[PD_INDEX(KERNEL_BASE)];
  setAttribute(entry3, PTE_PRESENT);
  setAttribute(entry3, PTE_READ_WRITE);
  setPhysicalFrame(entry3, (PhysicalAddress)pageTable3GB);

  PageDirectoryEntry *entry4 = &pageDirectory->entries[PD_INDEX(KERNEL_BASE + 4 * MB)];
  setAttribute(entry4, PTE_PRESENT);
  setAttribute(entry4, PTE_READ_WRITE);
  setPhysicalFrame(entry4, (PhysicalAddress)pageTable3GB2);

  // Switch to the page directory
  loadPageDirectory(pageDirectory);

  // Enable paging: Set PG (paging) bit 31 and PE (protection enable) bit 0 of CR0
  // asm volatile ("mov %CR0, %EAX; or $0x80000000, %EAX; mov %EAX, %CR0");
  enablePagination();

  printf("\nPage directory: %lx", pageDirectory);

  return true;
}
