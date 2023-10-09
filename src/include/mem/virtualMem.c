#include <stdint.h>
#include "virtualMem.h"
#include "mmu.h"

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
