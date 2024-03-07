#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <io.h>
#include <mem.h>
#include <virtualMem.h>
#include <syscallWrappers.h>
#include <MemoryManager.h>
#include <kernel/interrupts/pic.h>
#include <kernel/syscalls/syscalls.h>
#include <kernel/devices/KeyboardDevice.h>
#include <kernel/fileSystem/File.h>
#include <kernel/heap/kmalloc.h>
#include <kernel/tty/VirtualConsole.h>
#include <kernel/utils/kprintf.h>
#include <kernel/utils/datastructures/CircularQueue.h>
#include <kernel/filesystem/FileDescription.h>
#include <kernel/test.h>

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
