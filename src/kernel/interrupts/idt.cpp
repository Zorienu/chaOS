#include "idt.h"
#include <stdint.h>
#include "pic.h"
#include "../../kernel/irqHandler.h"

/*
 * The IDT (Interrupt Descriptor Table)
 */
IDTEntry32 idt32[MAX_IDT_ENTRIES];

/*
 * Interrupt Descriptor Table Register instance
 */
IDTR32 idtr32;

IRQHandler* irqHandlers[16];

void setIDTDescriptor(uint8_t entryNumber, void (*isr)(IntFrame32 *), uint8_t flags) {
  IDTEntry32 *descriptor = &idt32[entryNumber];

  descriptor->isrAddressLow = (uintptr_t)isr & 0xFFFF;          // Lower 16 bits of the ISR address
  descriptor->isrAddressHigh = ((uintptr_t)isr >> 16) & 0xFFFF; // Upper 16 bits of the ISR address
  descriptor->kernelCS = 0x08;                                 // Kernel code segment containing this isr
  descriptor->reserved = 0x0;                                  // Reserved for intel, set to 0
  descriptor->attributes = flags;                              // Type & attributes (INT_GATE, TRAP_GATE, etc.)
}

void initIDT(void) {
  idtr32.limit = (uint16_t)sizeof(idt32); // 256 entries * 8 bytes = 0x800
  idtr32.base = (uintptr_t)&idt32; // The address where the IDT is located

  // Load IDT to IDT register
  asm volatile("lidt %0" : : "memory"(idtr32));
}

__attribute__ ((interrupt)) void handleIRQWrapper(IntFrame32 *frame) {
  uint16_t isr = PIC::getISR();

  if (!isr) return;

  uint8_t irq = 0;
  for (uint8_t i = 0; i < 16; i++) {
    if (i == 2)
        continue;

    if (isr & (1 << i)) {
        irq = i;
        break;
    }
  }


  if (irqHandlers[irq]) {
      irqHandlers[irq]->handleIRQ();
      // Scheduler::stop_idling();
  }
 
  PIC::sendEOI(irq);
}

void registerIRQHandler(uint8_t irq, IRQHandler& handler) {
  // printf("\nRegistering irq number %d", irq);
  irqHandlers[irq] = &handler;
  setIDTDescriptor(PIC_IRQ_0_IDT_ENTRY + irq, handleIRQWrapper, INT_GATE_FLAGS);
}
