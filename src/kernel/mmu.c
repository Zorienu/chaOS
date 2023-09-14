#include "mmu.h"

typedef uint32_t PageDirectoryEntry;

/*
 * Map the first and last 4MiB of virtual addresses to first 4MiB of physical adresses
 * Here we use 4MiB pages for simplicity
 */
PageDirectoryEntry minimalPageDirectory[PAGE_DIRECTORY_ENTRIES]  __attribute__((aligned(PAGE_SIZE))) = {
  [0] = (0) | PAGE_SIZE_4_MB | READ_AND_WRITE_PAGE | PRESENT_PAGE,
  [PAGE_DIRECTORY_ENTRIES - 1] = (0) | PAGE_SIZE_4_MB | READ_AND_WRITE_PAGE | PRESENT_PAGE,
};

void loadPageDirectory(PageDirectoryEntry *pageDirectory) {
  asm volatile("mov %0, %%cr3" : : "r" (pageDirectory) : "memory");
}

void enablePagination() {
  loadPageDirectory(minimalPageDirectory);

  asm volatile("mov %cr4, %eax");
  asm volatile("or %0, %%eax" : : "i" (CR4_PSE)); // The "i" is for immediate integer operand
  asm volatile("mov %eax, %cr4");

  asm volatile("mov %cr0, %eax");
  asm volatile("or %0, %%eax" : : "i" (CR0_PG) ); // TODO: enabling with CR0_WP crashes
  asm volatile("mov %eax, %cr0");
}
