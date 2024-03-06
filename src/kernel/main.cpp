#include <stdint.h>
#include "../include/c/stdio.h"
#include "../include/mem/mem.h"
#include "../include/mem/virtualMem.h"
#include "../include/sys/syscallWrappers.h"
#include "syscalls/syscalls.h"
#include "../include/c/stdlib.h"
#include "interrupts/pic.h"
#include "../include/io/io.h"
#include "../include/mem/MemoryManager.h"
#include "devices/KeyboardDevice.h"
#include "fileSystem/File.h"
#include "test.h"
#include "heap/kmalloc.h"
#include "tty/VirtualConsole.h"
#include "utils/kprintf.h"
#include "utils/datastructures/CircularQueue.h"
#include "filesystem/FileDescription.h"
#include "../include/c/string.h"
#include "../include/c/math.h"

// https://stackoverflow.com/questions/329059/what-is-gxx-personality-v0-for
void *__gxx_personality_v0;
void *_Unwind_Resume;

/*
 * Entry point of the operating system, called from bootmain.c
 * Add 'extern "C"' to avoid mangling
 */
extern "C" void OSStart() {
  initVideo();
  printMemoryMap();

  initIDT();

  setIDTDescriptor(0x80, syscallDispatcher, INT_GATE_USER_FLAGS);

  PIC::disableAll();
  PIC::initializePIC();

  setIDTDescriptor(PIC_IRQ_0_IDT_ENTRY, PIC::pitIRQ0Handler, INT_GATE_FLAGS);

  PIC::configurePIT(0, 2, 1193180 / 100);
  
  PIC::enable(PIC_IRQ_TIMER);
  
  // Call before using the "new" operator
  kmallocInit();
  pritnfKmallocInformation();

  new KeyboardDevice();

  asm volatile("sti");

  VirtualConsole::initialize();
  VirtualConsole *vc = new VirtualConsole(0);
  VirtualConsole *vc2 = new VirtualConsole(1);
  vc->switchTo(0);
  
  FileDescription *fd = FileDescription::create(*vc);
  char buffer[16];
  memset(buffer, 0x0, sizeof(buffer));
  int nRead = 0;

  kprintf("\n0 / 512 = %d", Math::ceilDiv(0, 512));
  kprintf("\n1 / 512 = %d", Math::ceilDiv(1, 512));
  kprintf("\n511 / 512 = %d", Math::ceilDiv(511, 512));
  kprintf("\n512 / 512 = %d", Math::ceilDiv(512, 512));
  kprintf("\n513 / 512 = %d", Math::ceilDiv(513, 512));

  for(;;) {
    if (nRead < sizeof(buffer))
      nRead += vc->read(*fd, (uint8_t *)&buffer[nRead], sizeof(buffer));

    for (int i = 0; i < nRead; i++) {
      if (buffer[i] == '\n') { 
        kprintf("\nRead buffer:%s \n", buffer);
        memset(buffer, 0x0, sizeof(buffer));
        nRead = 0;
        break;
      };
    }
  }


  while(1);
}
