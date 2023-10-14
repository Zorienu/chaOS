#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MEMORY_BITMAP_ADDRESS 0x30000;

/*
 * Print the memory map obtained from 0xE820 BIOS int
 */
void printMemoryMap();

/* 
 * Initialize the physical memory manager
 */
void initializePhysicalMemoryManager(uint32_t kernelEnd);

/* 
 * Initialize the virtual memory manager
 */
bool initializeVirtualMemoryManager();
