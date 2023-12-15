#include <stdint.h>
#include "../include/c/string.h"
#include "../include/c/stdio.h"
#include "../include/mem/mem.h"
#include "../include/mem/virtualMem.h"
#include "../include/sys/syscallWrappers.h"
#include "../include/syscalls/syscalls.h"
#include "../include/c/stdlib.h"
#include "../include/interrupts/pic.h"
#include "../include/io/io.h"
#include "../include/mem/MemoryManager.h"
#include "test.h"
#include "heap/kmalloc.h"

/*
 * Entry point of the operating system, called from bootmain.c
 * Add 'extern "C"' to avoid mangling
 */
extern "C" void OSStart() {
  initVideo();
  printMemoryMap();

  initIDT();

  setIDTDescriptor(0x80, syscallDispatcher, INT_GATE_USER_FLAGS);

  disablePIC();
  initializePIC();

  setIDTDescriptor(PIC_IRQ_0_IDT_ENTRY, pitIRQ0Handler, INT_GATE_FLAGS );
  setIDTDescriptor(0x21, keyboardIRQ1Handler, INT_GATE_FLAGS);

  configurePIT(0, 2, 1193180 / 100);
  
  enableIRQ(PIC_IRQ_KEYBOARD);
  enableIRQ(PIC_IRQ_TIMER);
  
  asm volatile("sti");

  pritnfKmallocInformation();
  kmallocInit();
  printf("\nFirst kmalloc: %lx", kmalloc(5));
  printf("\nSecond kmalloc: %lx", kmalloc(10));
  TestClass *counter = new TestClass();
  counter->setCounter(10);

  printf("\nCounter: %d", counter->getCounter());
  counter->incrementCounter();
  printf("\nCounter: %d", counter->getCounter());

  MemoryManager *mem = (MemoryManager *)0x30000; 

  mem->print();

  while(1);

  /*
  int32_t result = syscallTestWrapper();
  printf("\nTest syscall result: %d", result);
  printf("\nSyscall dispatcher address: %lx", syscallDispatcher); 

  void *ptr = malloc(5000);
  printf("\nFirst malloc-ed block: %lx", ptr);
  
  uint32_t *ptr2 = malloc(10 * sizeof(uint32_t));
  printf("\nSecond malloc-ed block: %lx", ptr2);

  free(ptr2);

  mapPage(0x7EE0000, 0x7ED0000);
  uint8_t *test = (uint8_t *)0x7EE0000;
  *test = 0xA2;

  printf("\nMap far enough page: %x", *test);

  printVirtualAddressInfo(0x7EE0000);
  */
}
