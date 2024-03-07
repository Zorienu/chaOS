#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <memLayout.h>
#include <mem.h>
#include <virtualMem.h>
#include <stdio.h>
#include <string.h>

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


struct PhysicalMemory {
  uint32_t bitmap[MAX_BLOCKS_AMOUNT / 32];
};

struct PhysicalMemory *physicalMemory = (struct PhysicalMemory *)MEMORY_BITMAP_ADDRESS;
uint32_t availableBlocks = 0;


void printMemoryMap() {
  uint32_t SMAPNumEntries = *(uint32_t *)SMAP_NUM_ENTRIES_ADDRESS;
  SMAP_entry_t *SMAPEntry = (SMAP_entry_t *)SMAP_ENTRIES_ADDRESS;

  uint32_t availableBlocks = 0;
  uint32_t reservedBlocks = 0;

  // Available:
  // base address:      0x0, length: 0x9FC00
  // base address: 0x100000, length: 0x7EE0000
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
  printf("\nPhysicalMemory: %lx", physicalMemory);
  printf("\nPhysicalMemory value: %lx %lb", &physicalMemory->bitmap[8], physicalMemory->bitmap[8]);
}

void initializeBlock(uint32_t address) {
  uint32_t block = address / PAGE_SIZE;
  uint32_t *entry = &physicalMemory->bitmap[block / 32];
  uint32_t bit = block % 32;

  *entry = *entry | (1 << bit);

  availableBlocks++;
}

void deinitializeBlock(uint32_t address) {
  uint32_t block = address / PAGE_SIZE;
  uint32_t *entry = &physicalMemory->bitmap[block / 32];
  uint32_t bit = block % 32;

  *entry = *entry & ~(1 << bit);

  availableBlocks--;
}

void *allocateBlock() {
  uint32_t entryIdx = 0;

  // Find an entry with at least 1 free block
  while (!physicalMemory->bitmap[entryIdx]) entryIdx++;

  uint32_t entry = physicalMemory->bitmap[entryIdx];

  // The bit within the entry
  uint8_t bit = 0;

  // Found a free page within the entry
  while (!(entry & (1 << bit))) bit++;

  uint32_t address = (entryIdx * 32 + bit) * PAGE_SIZE;

  deinitializeBlock(address);

  // Calculate the address of the found block
  return (void *)address;
}

void initializePhysicalMemoryManager(uint32_t kernelEnd) {
  uint32_t page = 0;

  physicalMemory = (struct PhysicalMemory *)MEMORY_BITMAP_ADDRESS;

  // Initialize all memory as used
  memset(physicalMemory->bitmap, 0, MAX_BLOCKS_AMOUNT / 32);

  uint32_t SMAPNumEntries = *(uint32_t *)SMAP_NUM_ENTRIES_ADDRESS;
  SMAP_entry_t *SMAPEntry = (SMAP_entry_t *)SMAP_ENTRIES_ADDRESS;

  for (int i = 0; i < SMAPNumEntries; i++) {
    if (SMAPEntry->type == AVAILABLE) {
      // Allocate only available memory
      uint32_t alignedBaseAddress = ALIGN_ADDRESS_UP(SMAPEntry->baseAddress);

      for (int offset = 0; offset + PAGE_SIZE < SMAPEntry->length; offset += PAGE_SIZE) 
        if ((alignedBaseAddress + offset) > kernelEnd)
          initializeBlock(alignedBaseAddress + offset);
    }

    // Continue with the next entry in the table
    SMAPEntry++;
  }
}

void map4MBPage(PageTable *pageTable, PhysicalAddress physicalFrame, VirtualAddress virtualAddress) {
  for (uint32_t i = 0; i < PAGES_PER_TABLE; i++, physicalFrame += PAGE_SIZE, virtualAddress += PAGE_SIZE) {
    // Create new page entry
    PageTableEntry entry = 0;

    setAttribute(&entry, PTE_PRESENT);
    setAttribute(&entry, PTE_READ_WRITE);
    setPhysicalFrame(&entry, physicalFrame);

    // Add page entry to the page table
    pageTable->entries[getPageTableIndex(virtualAddress)] = entry;
  }
}

bool initializeVirtualMemoryManager() {
  PageDirectory *pageDirectory = (PageDirectory *)allocateBlock();

  // Out of memory
  if (!pageDirectory) return false;
  // printf("\nPage directory: %lx", pageDirectory);

  memset(pageDirectory, 0, sizeof(PageDirectory));

  for (int i = 0; i < TABLES_PER_DIRECTORY; i++) {
    pageDirectory->entries[i] = PTE_READ_WRITE; // Supervisor, read/write, not present
  }

  // For identity map from 0x0 to 4MB
  PageTable *identityPageTable = (PageTable *)allocateBlock();
  // printf("\nIdentity page table: %lx", identityPageTable);
  // Out of memory
  if (!identityPageTable) return false;
  memset(identityPageTable, 0, sizeof(PageTable));
  // Fill identity mapped page table from 0x0 to 4MB
  map4MBPage(identityPageTable, 0x0, 0x0);

  // For identity map from 4MB to 8MB
  PageTable *identityPageTable2 = (PageTable *)allocateBlock();
  // printf("\nIdentity page table 2: %lx", identityPageTable2);
  // Out of memory
  if (!identityPageTable2) return false;
  memset(identityPageTable2, 0, sizeof(PageTable));
  // Fill identity mapped page table from 4MB to 8MB
  map4MBPage(identityPageTable2, 4 * MB, 4 * MB);


  // For mapping from KERNEL_BASE to 0x0 (0x0 to 4MB)
  PageTable *pageTable3GB = (PageTable *)allocateBlock();
  // printf("\nPage table 3GB: %lx", pageTable3GB);
  // Out of memory
  if (!pageTable3GB) return false;
  memset(pageTable3GB, 0, sizeof(PageTable));
  // Fill page table for mapping 3GB to 0x0 (where the kernel resides)
  map4MBPage(pageTable3GB, 0x0, KERNEL_BASE);

  // For mapping from KERNEL_BASE to 0x0 (0x0 to 4MB)
  PageTable *pageTable3GB2 = (PageTable *)allocateBlock();
  // printf("\nPage table 3GB 2: %lx", pageTable3GB2);
  // Out of memory
  if (!pageTable3GB2) return false;
  memset(pageTable3GB2, 0, sizeof(PageTable));
  // Fill page table for mapping (3GB + 4MB) to 4MB (where the kernel resides)
  map4MBPage(pageTable3GB2, 4 * MB, KERNEL_BASE + 4 * MB);


  PageDirectoryEntry *entry1 = &pageDirectory->entries[getPageDirectoryIndex(0x0)];
  setAttribute(entry1, PTE_PRESENT);
  setAttribute(entry1, PTE_READ_WRITE);
  setPhysicalFrame(entry1, (PhysicalAddress)identityPageTable);

  PageDirectoryEntry *entry2 = &pageDirectory->entries[getPageDirectoryIndex(4 * MB)];
  setAttribute(entry2, PTE_PRESENT);
  setAttribute(entry2, PTE_READ_WRITE);
  setPhysicalFrame(entry2, (PhysicalAddress)identityPageTable2);



  PageDirectoryEntry *entry3 = &pageDirectory->entries[getPageDirectoryIndex(KERNEL_BASE)];
  setAttribute(entry3, PTE_PRESENT);
  setAttribute(entry3, PTE_READ_WRITE);
  setPhysicalFrame(entry3, (PhysicalAddress)pageTable3GB);

  PageDirectoryEntry *entry4 = &pageDirectory->entries[getPageDirectoryIndex(KERNEL_BASE + 4 * MB)];
  setAttribute(entry4, PTE_PRESENT);
  setAttribute(entry4, PTE_READ_WRITE);
  setPhysicalFrame(entry4, (PhysicalAddress)pageTable3GB2);

  // Switch to the page directory
  loadPageDirectory(pageDirectory);

  // Enable paging: Set PG (paging) bit 31 and PE (protection enable) bit 0 of CR0
  enablePagination();

  return true;
}

