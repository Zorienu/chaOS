#include <stddef.h>
#include <stdint.h>

/*
 * Print the memory map obtained from 0xE820 BIOS int
 */
void printMemoryMap();

/* 
 * Initialize the physical memory manager
 */
void initializePhysicalMemoryManager();

/* 
 * Initialize the virtual memory manager
 */
void initializeVirtualMemoryManager();
