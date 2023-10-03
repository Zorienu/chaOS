#include "mmu.h"
#include "memLayout.h"

typedef uint32_t PageDirectoryEntry;

/*
 * Map the first and last 4MiB of virtual addresses to first 4MiB of physical adresses
 * Here we use 4MiB pages for simplicity
 */
PageDirectoryEntry entryPageDirectory[PAGE_DIRECTORY_ENTRIES]  __attribute__((aligned(PAGE_SIZE))) = {
  // 0x0 to 0x400000 - 1
  [0] = (0) | PAGE_SIZE_4_MB | READ_AND_WRITE_PAGE | PRESENT_PAGE,
  // 0x400000 to 0x800000 - 1
  [1] = (0x400000) | PAGE_SIZE_4_MB | READ_AND_WRITE_PAGE | PRESENT_PAGE,
  // 0x0 to 0x400000 - 1
  [KERNEL_BASE >> PAGE_DIRECTORY_SHIFT] = (0) | PAGE_SIZE_4_MB | READ_AND_WRITE_PAGE | PRESENT_PAGE,
  // 0x400000 to 0x800000 - 1
  [(KERNEL_BASE >> PAGE_DIRECTORY_SHIFT) + 1] = (0x400000) | PAGE_SIZE_4_MB | READ_AND_WRITE_PAGE | PRESENT_PAGE,
};

void loadPageDirectory(PageDirectoryEntry *pageDirectory) {
  asm volatile("mov %0, %%cr3" : : "r" (pageDirectory) : "memory");
}
