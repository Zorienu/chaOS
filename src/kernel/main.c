#include <stdint.h>
#include "../include/c/string.h"
#include "../include/c/stdio.h"
#include "../include/mem/mem.h"
#include "../include/mem/virtualMem.h"
#include "../include/interrupts/idt.h"
#include "../include/sys/syscallWrappers.h"
#include "../include/syscalls/syscalls.h"

void OSStart() {
  initVideo();
  printMemoryMap();

  setIDTDescriptor(0x80, syscallDispatcher, 0xEE);

  initIDT();


  int32_t result = syscallTestWrapper();
  printf("\nTest syscall result: %d", result);
  printf("\nSyscall dispatcher address: %lx", syscallDispatcher); 

  mapPage(0x7EE0000, 0x7ED0000);
  uint8_t *test = (uint8_t *)0x7EE0000;
  *test = 0xA2;

  printf("\nMap far enough page: %x", *test);

  printVirtualAddressInfo(0x7EE0000);
}
