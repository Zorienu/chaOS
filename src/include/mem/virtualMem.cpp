#include <stdint.h>
#include "virtualMem.h"
#include "mmu.h"
#include "mem.h"
#include "../c/string.h"
#include "../c/stdio.h"

/*
 * Used to map a predefined virtual address to any physical address 
 * Useful for creating PageTables on no identity mapped physical addresses
 */
__attribute__((section(".kernel_quickmap_page"))) PageTable quickmapPageAddress;
__attribute__((section(".kernel_quickmap_page"))) PageDirectory quickmapPageDirectoryAddress;
__attribute__((section(".kernel_quickmap_page"))) PageTable quickmapPageTableAddress;

void setAttribute(PageTableEntry *entry, PAGE_TABLE_FLAGS attribute) {
  *entry |= attribute;
}

void setPhysicalFrame(PageTableEntry *entry, PhysicalAddress frame) {
  // Ox7FF = bits sets from bit 0 to bit 11 (flag bits)
  // Preserve flags and remove old frame
  // Set new frame
  *entry = (*entry & 0x7FF) | frame;
}

void loadPageDirectory(PageDirectory *pageDirectory) {
  asm volatile("mov %0, %%cr3" : : "r" (pageDirectory) : "memory");
}

void enablePagination() {
  asm volatile("mov %cr4, %eax");
  asm volatile("or %0, %%eax" : : "i" (CR4_PSE)); // The "i" is for immediate integer operand
  asm volatile("mov %eax, %cr4");

  asm volatile("mov %cr0, %eax");
  asm volatile("or %0, %%eax" : : "i" (CR0_PG) ); // TODO: enabling with CR0_WP crashes
  asm volatile("mov %eax, %cr0");
}

/*
 * Reload the CR3 control register
 */
void reloadCR3() {
  asm volatile("mov %cr3, %ecx");
  asm volatile("mov %ecx, %cr3");
}

PageDirectory *getPageDirectory() {
  PageDirectory *cr3 = 0;

  asm volatile("mov %%cr3, %%eax" : "=a" (cr3));

  return cr3;
}

uint32_t getPageDirectoryIndex(VirtualAddress virtualAddress) {
  return virtualAddress >> 22;
}

uint32_t getPageTableIndex(VirtualAddress virtualAddress) {
  return virtualAddress >> 12 & (0x3FF);
}

PageTable *getPagePhysicalAddress(PageDirectoryEntry *entry) {
  // Remove the 12 bits flags
  return (PageTable *)(*entry & ~0xFFF);
}

VirtualAddress quickmapPage(PhysicalAddress physicalAddress) {
  return mapPage((VirtualAddress)&quickmapPageAddress, physicalAddress);
}

PageDirectory *quickmapPageDirectory(PageDirectory *pageDirectory) {
  PageDirectory *currentPageDirectory = getPageDirectory();

  PageDirectoryEntry *pageDirectoryEntry = &currentPageDirectory->entries[getPageDirectoryIndex((VirtualAddress)&quickmapPageDirectoryAddress)];
  PageTable *pageTable = getPagePhysicalAddress(pageDirectoryEntry);

  PageTableEntry *pageTableEntry = &pageTable->entries[getPageTableIndex((VirtualAddress)&quickmapPageDirectoryAddress)];

  setAttribute(pageTableEntry, PTE_PRESENT);
  setAttribute(pageTableEntry, PTE_READ_WRITE);
  setPhysicalFrame(pageTableEntry, (PhysicalAddress)pageDirectory);

  reloadCR3();

  return &quickmapPageDirectoryAddress;
}

PageTable *quickmapPageTable(PageTable *pageTable) {
  PageDirectory *currentPageDirectory = getPageDirectory();

  PageDirectoryEntry *pageDirectoryEntry = &currentPageDirectory->entries[getPageDirectoryIndex((VirtualAddress)&quickmapPageTableAddress)];
  PageTable *quickmapPageTable = getPagePhysicalAddress(pageDirectoryEntry);

  PageTableEntry *pageTableEntry = &quickmapPageTable->entries[getPageTableIndex((VirtualAddress)&quickmapPageTableAddress)];

  setAttribute(pageTableEntry, PTE_PRESENT);
  setAttribute(pageTableEntry, PTE_READ_WRITE);
  setPhysicalFrame(pageTableEntry, (PhysicalAddress)pageTable);

  reloadCR3();

  return &quickmapPageTableAddress;
}

VirtualAddress mapPage(VirtualAddress virtualAddress, PhysicalAddress physicalAddress) {
  PageDirectory *currentPageDirectory = quickmapPageDirectory(getPageDirectory());

  PageDirectoryEntry *pageDirectoryEntry = &currentPageDirectory->entries[getPageDirectoryIndex(virtualAddress)];
  PageTable *pageTable = getPagePhysicalAddress(pageDirectoryEntry);

  // TODO: change to test attribute PDE_PRESENT for the page directory entry
  // TODO: implement quickmap pd and pt as in SerenetyOS
  // https://github.dev/SerenityOS/serenity/blob/master/Kernel/Memory/MemoryManager.cpp - MemoryManager::pte
  if (!pageTable) {
    void *allocatedBlock = allocateBlock();
    pageTable = quickmapPageTable((PageTable *)allocatedBlock);
    memset((void *)pageTable, 0x0, sizeof(PageTable));

    setAttribute(pageDirectoryEntry, PTE_PRESENT);
    setAttribute(pageDirectoryEntry, PTE_READ_WRITE);
    setPhysicalFrame(pageDirectoryEntry, (PhysicalAddress)allocatedBlock);

    printf("\nEntered here - page Table: %lx, v address: %lx, getPageTableIndex: %lx", pageTable, allocatedBlock, getPageTableIndex(virtualAddress));
  }

  PageTableEntry *pageTableEntry = &pageTable->entries[getPageTableIndex(virtualAddress)];

  setAttribute(pageTableEntry, PTE_PRESENT);
  setAttribute(pageTableEntry, PTE_READ_WRITE);
  setPhysicalFrame(pageTableEntry, physicalAddress);

  reloadCR3();

  return virtualAddress;
}

void printVirtualAddressInfo(VirtualAddress virtualAddress) {
  printf("\n=== Information for virtual address: %lx ===", virtualAddress);
  printf("\nCurrently active page directory: %lx", getPageDirectory());
  printf("\nPage directory index: %lx, (offset within the PD: %lx)", getPageDirectoryIndex(virtualAddress), getPageDirectoryIndex(virtualAddress) * 4);
  printf("\nPage table index: %lx, (offset within the PT: %lx)", getPageTableIndex(virtualAddress), getPageTableIndex(virtualAddress) * 4);

  PageDirectory *currentPageDirectory = quickmapPageDirectory(getPageDirectory());

  printf("\nCurrent page directory: %lx", currentPageDirectory);

  PageDirectoryEntry *pageDirectoryEntry = &currentPageDirectory->entries[getPageDirectoryIndex(virtualAddress)];
  printf("\nPage directory entry - address: %lx, value: %lx, block: %lx", pageDirectoryEntry, *pageDirectoryEntry, getPagePhysicalAddress(pageDirectoryEntry));

  PageTable *pageTable = quickmapPageTable(getPagePhysicalAddress(pageDirectoryEntry));
  printf("\nPage table: %lx", pageTable);

  PageTableEntry *pageTableEntry = &pageTable->entries[getPageTableIndex(virtualAddress)];
  printf("\nPage table entry - address: %lx, value: %lx, block: %lx", pageTableEntry, *pageTableEntry, getPagePhysicalAddress(pageTableEntry));
}
