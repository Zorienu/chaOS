#include <stdint.h>
#include "../include/c/string.h"
#include "../include/c/stdio.h"
#include "../include/mem/mem.h"

extern uint32_t *kernelEnd;

#define NELEM 10

void OSStart() {
  initVideo();
  printMemoryMap();
  
  // Test printf implementation
  printf("\nFormatted %d %i %x %p %o %hd %hi %hhu %hhd", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (uint8_t)20, (int8_t)-10);
  printf("\nFormatted %lx %ld %lld %llx",  0xdeadbeeful, -100000000l, 10200300400ll, 0xdeadbeeffeebdaedull);
  printf("\nLength: %d", strlen("hola"));
}
