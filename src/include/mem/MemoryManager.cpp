#include <stdint.h>
#include "../c/string.h"
#include "MemoryManager.h"
#include "PageDirectory.h"
#include "PhysicalAddress.h"
#include "PageTableEntry.h"
#include "PageTable.h"
#include "mem.h"
#include "mmu.h"
#include "memLayout.h"
#include "../c/stdio.h"

#define MEMORY_BITMAP_ADDRESS 0x30000;


void MemoryManager::initialize(uint32_t kernelEnd) {
  /*
   * Physical memory initialization
   */
  uint32_t page = 0;

  // NOT needed?
  // physicalMemory = (struct PhysicalMemory *)MEMORY_BITMAP_ADDRESS;

  // Initialize all memory as used
  memset(bitmap, 0, MAX_BLOCKS_AMOUNT / 32);

  uint32_t SMAPNumEntries = *(uint32_t *)SMAP_NUM_ENTRIES_ADDRESS;
  SMAP_entry_t *SMAPEntry = (SMAP_entry_t *)SMAP_ENTRIES_ADDRESS;

  for (int i = 0; i < SMAPNumEntries; i++) {
    if (SMAPEntry->type == AVAILABLE) {
      // Allocate only available memory
      PhysicalAddress2 alignedBaseAddress = PhysicalAddress2(SMAPEntry->baseAddress).alignUp();

      for (int offset = 0; offset + PAGE_SIZE < SMAPEntry->length; offset += PAGE_SIZE) 
        if (alignedBaseAddress.offset(offset).get() > kernelEnd)
          initializeBlock(alignedBaseAddress.offset(offset));
    }

    // Continue with the next entry in the table
    SMAPEntry++;
  }

  /*
   * Virtual memory initialization
   */
  PageDirectory2 *pageDirectory = (PageDirectory2 *)allocateBlock();
  //
  // Out of memory
  if (!pageDirectory) return;
  // printf("\nPage directory: %lx", pageDirectory);

  memset(pageDirectory, 0, sizeof(PageDirectory2));

  for (int i = 0; i < TABLES_PER_DIRECTORY; i++) 
    pageDirectory->entries[i].setWritable(true); // Supervisor, read/write, not present


  // For identity map from 0x0 to 4MB
  PageTable2 *identityPageTable = (PageTable2 *)allocateBlock();
  // printf("\nIdentity page table: %lx", identityPageTable);
  // Out of memory
  if (!identityPageTable) return;
  memset(identityPageTable, 0, sizeof(PageTable2));
  // Fill identity mapped page table from 0x0 to 4MB
  map4MBPage(identityPageTable, PhysicalAddress2(0x0), VirtualAddress2(0x0));

  // For identity map from 4MB to 8MB
  PageTable2 *identityPageTable2 = (PageTable2 *)allocateBlock();
  // printf("\nIdentity page table 2: %lx", identityPageTable2);
  // Out of memory
  if (!identityPageTable2) return;
  memset(identityPageTable2, 0, sizeof(PageTable2));
  // Fill identity mapped page table from 4MB to 8MB
  map4MBPage(identityPageTable2, PhysicalAddress2(4 * MB), VirtualAddress2(4 * MB));


  // For mapping from KERNEL_BASE to 0x0 (0x0 to 4MB)
  PageTable2 *pageTable3GB = (PageTable2 *)allocateBlock();
  // printf("\nPage table 3GB: %lx", pageTable3GB);
  // Out of memory
  if (!pageTable3GB) return;
  memset(pageTable3GB, 0, sizeof(PageTable2));
  // Fill page table for mapping 3GB to 0x0 (where the kernel resides)
  map4MBPage(pageTable3GB, PhysicalAddress2(0x0), VirtualAddress2(KERNEL_BASE));

  // For mapping from KERNEL_BASE to 0x0 (0x0 to 4MB)
  PageTable2 *pageTable3GB2 = (PageTable2 *)allocateBlock();
  // printf("\nPage table 3GB 2: %lx", pageTable3GB2);
  // Out of memory
  if (!pageTable3GB2) return;
  memset(pageTable3GB2, 0, sizeof(PageTable2));
  // Fill page table for mapping (3GB + 4MB) to 4MB (where the kernel resides)
  map4MBPage(pageTable3GB2, PhysicalAddress2(4 * MB), VirtualAddress2(KERNEL_BASE + 4 * MB));


  PageDirectoryEntry2 *entry1 = &pageDirectory->entries[getPageDirectoryIndex(0x0)];
  entry1->setPresent(true);
  entry1->setWritable(true);
  entry1->setPhysicalFrame((uintptr_t)identityPageTable);

  PageDirectoryEntry2 *entry2 = &pageDirectory->entries[getPageDirectoryIndex(4 * MB)];
  entry2->setPresent(true);
  entry2->setWritable(true);
  entry2->setPhysicalFrame((uintptr_t)identityPageTable2);



  PageDirectoryEntry2 *entry3 = &pageDirectory->entries[getPageDirectoryIndex(KERNEL_BASE)];
  entry3->setPresent(true);
  entry3->setWritable(true);
  entry3->setPhysicalFrame((uintptr_t)pageTable3GB);

  PageDirectoryEntry2 *entry4 = &pageDirectory->entries[getPageDirectoryIndex(KERNEL_BASE + 4 * MB)];
  entry4->setPresent(true);
  entry4->setWritable(true);
  entry4->setPhysicalFrame((uintptr_t)pageTable3GB2);

  // Switch to the page directory
  loadPageDirectory2(pageDirectory);

  // Enable paging: Set PG (paging) bit 31 and PE (protection enable) bit 0 of CR0
  enablePagination2();
}

void MemoryManager::initializeBlock(PhysicalAddress2 pa) {
  uint32_t block = pa.get() / PAGE_SIZE;
  uint32_t *entry = &bitmap[block / 32];
  uint32_t bit = block % 32;

  *entry = *entry | (1 << bit);

  availableBlocks++;
}

void MemoryManager::deinitializeBlock(PhysicalAddress2 address) {
  uint32_t block = address.get() / PAGE_SIZE;
  uint32_t *entry = &bitmap[block / 32];
  uint32_t bit = block % 32;

  *entry = *entry & ~(1 << bit);

  availableBlocks--;
}

void *MemoryManager::allocateBlock() {
  uint32_t entryIdx = 0;

  // Find an entry with at least 1 free block
  while (!bitmap[entryIdx]) entryIdx++;

  uint32_t entry = bitmap[entryIdx];

  // The bit within the entry
  uint8_t bit = 0;

  // Found a free page within the entry
  while (!(entry & (1 << bit))) bit++;

  PhysicalAddress2 pa = PhysicalAddress2((entryIdx * 32 + bit) * PAGE_SIZE);

  deinitializeBlock(pa);

  // Calculate the address of the found block
  return (void *)pa.get();
}

void MemoryManager::map4MBPage(PageTable2 *pageTable, PhysicalAddress2 physicalFrame, VirtualAddress2 va) {
  uint32_t physicalFrameAddress = physicalFrame.get();
  uint32_t virtualAddress = va.get();


  for (uint32_t i = 0; i < PAGES_PER_TABLE; i++, physicalFrameAddress += PAGE_SIZE, virtualAddress += PAGE_SIZE) {
    // Create new page entry
    PageTableEntry2 *entry = { 0 };

    entry->setPresent(true);
    entry->setWritable(true);
    entry->setUserAllowed(false);
    entry->setPhysicalFrame(physicalFrameAddress);

    // Add page entry to the page table
    pageTable->entries[getPageTableIndex(virtualAddress)] = *entry;
  }
}

void MemoryManager::loadPageDirectory2(PageDirectory2 *pd) {
  asm volatile("mov %0, %%cr3" : : "r" (pd) : "memory");
}

void MemoryManager::enablePagination2() {
  asm volatile("mov %cr4, %eax");
  asm volatile("or %0, %%eax" : : "i" (CR4_PSE)); // The "i" is for immediate integer operand
  asm volatile("mov %eax, %cr4");

  asm volatile("mov %cr0, %eax");
  asm volatile("or %0, %%eax" : : "i" (CR0_PG) ); // TODO: enabling with CR0_WP crashes
  asm volatile("mov %eax, %cr0");
}

void MemoryManager::print() {
  printf("\nPhysicalMemory MemoryManager: %lx", &bitmap);
  printf("\nPhysicalMemory MemoryManager: %lx %lb", &bitmap[8], bitmap[8]);
}
