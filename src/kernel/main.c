#include <stdint.h>
#include "../include/c/string.h"
#include "../include/c/stdio.h"
#include "../include/mem/mem.h"
#include "../include/mem/virtualMem.h"

extern uint32_t *kernelEnd;

#define NELEM 10

void OSStart() {
  initVideo();
  printMemoryMap();

  PageDirectory *currentPageDirectory = getPageDirectory();
  printf("\nCurrently active page directory: %lx", currentPageDirectory);

  PageTable *firstKernelPageTable = allocateBlock();
  memset(firstKernelPageTable, 0, sizeof(PageTable));

  printf("\nFirst allocation from kernel: %lx", firstKernelPageTable);

  // 0x401000 -> firstKernelPageTable
  uint32_t virtualAddress = 4 * MB + 0x1000;

  mapPage(virtualAddress, (PhysicalAddress)firstKernelPageTable);

  // Get the 4MB + 4096 bytes PageTableEntry
  printf("\nVirtual address: %lx, Physical address: %lx", virtualAddress, firstKernelPageTable);

  uint32_t *test = (uint32_t *)virtualAddress;
  printf("\nBefore assignment: %ld", *test); 
  *test = 0x1234;
  printf("\nAfter assignment: %ld", *test);

  // 0x401FFC -> firstKernelPageTable + FFC
  uint32_t *test2 = (uint32_t *)(virtualAddress + 0x1000 - 4);
  printf("\nBefore assignment: %ld", *test2);
  *test2 = 0x3412;
  printf("\nAfter assignment: %ld", *test2);

  
  // Test printf implementation
  printf("\nFormatted %d %i %x %p %o %hd %hi %hhu %hhd", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (uint8_t)20, (int8_t)-10);
  printf("\nFormatted %lx %ld %lld %llx",  0xdeadbeeful, -100000000l, 10200300400ll, 0xdeadbeeffeebdaedull);
}
