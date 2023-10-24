#include <stdint.h>
#include "virtualMem.h"
#include "mmu.h"

/*
 * Used to map a predefined virtual address to any physical address 
 * Useful for creating PageTables on no identity mapped physical addresses
 */
__attribute__((section(".kernel_quickmap_page"))) PageTable quickmapPageAddress;

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
 * Store current page directory for kernel to use 
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

PageTable *getPagePhysicalAddress(PageDirectoryEntry *entry) {
  // Remove the 12 bits flags
  return (PageTable *)(*entry & ~0xFFF);
}

void mapPage(VirtualAddress virtualAddress, PhysicalAddress physicalAddress) {
  PageDirectory *currentPageDirectory = getPageDirectory();

  PageDirectoryEntry *pageDirectoryEntry = &currentPageDirectory->entries[PD_INDEX(virtualAddress)];
  PageTable *pageTable = getPagePhysicalAddress(pageDirectoryEntry);
  PageTableEntry *pageTableEntry = &pageTable->entries[PT_INDEX(virtualAddress)];

  setPhysicalFrame(pageTableEntry, physicalAddress);
}
