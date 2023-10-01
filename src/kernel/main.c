#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "mem.h"
#include "mmu.h"
#include "mem.h"
#include "memLayout.h"

extern uint32_t *kernelEnd;

#define NELEM 10

void OSStart() {
  initVideo();
  initializePhysicalMemoryManager();
  
  // Test printf implementation
  printf("\nFormatted %d %i %x %p %o %hd %hi %hhu %hhd", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (uint8_t)20, (int8_t)-10);
  printf("\nFormatted %lx %ld %lld %llx",  0xdeadbeeful, -100000000l, 10200300400ll, 0xdeadbeeffeebdaedull);
}
