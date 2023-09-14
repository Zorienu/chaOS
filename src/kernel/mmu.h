#include <stdint.h>

// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_WP          0x00010000      // Write Protect
#define CR0_PG          0x80000000      // Paging

#define CR4_PSE         0x00000010      // Page size extension

#define PAGE_SIZE 4096
#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024

#define PAGE_SIZE_4_MB      1 << 7
#define READ_AND_WRITE_PAGE 1 << 2
#define PRESENT_PAGE        1 << 0

/*
 * Enable pagination by:
 * - Loading the CR3 register with a barely defined page directory (using 4MiB pages)
 * - Turning on PSE (Page Size Extension) for using 4MiB pages by setting the second bit in the CR4 register
 * - Enabling paging and write protection setting the CR0 register
 */
void enablePagination();
