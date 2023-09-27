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
  puts("\ndone");
}
