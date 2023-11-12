#include <stdint.h>
#include "../include/c/string.h"
#include "../include/c/stdio.h"
#include "../include/mem/mem.h"
#include "../include/mem/virtualMem.h"
#include "../include/interrupts/idt.h"
#include "../include/sys/syscallWrappers.h"
#include "../include/syscalls/syscalls.h"
#include "../include/c/stdlib.h"

void OSStart() {
  initVideo();
  printMemoryMap();
  allocateBlock();

  setIDTDescriptor(0x80, syscallDispatcher, INT_GATE_USER_FLAGS);

  initIDT();


  int32_t result = syscallTestWrapper();
  printf("\nTest syscall result: %d", result);
  printf("\nSyscall dispatcher address: %lx", syscallDispatcher); 

  void *ptr = malloc(4);
  printf("\nFirst malloc-ed block: %lx", ptr);
  
  void *ptr2 = malloc(8);
  printf("\nSecond malloc-ed block: %lx", ptr2);

  mapPage(0x7EE0000, 0x7ED0000);
  uint8_t *test = (uint8_t *)0x7EE0000;
  *test = 0xA2;

  printf("\nMap far enough page: %x", *test);

  printVirtualAddressInfo(0x7EE0000);
}
