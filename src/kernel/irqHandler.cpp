#include "irqHandler.h"
#include "../include/interrupts/pic.h"
#include "../include/interrupts/idt.h"

IRQHandler::IRQHandler(uint8_t irq) : _irq_number(irq) {
  registerIRQHandler(irq, *this);
}

void IRQHandler::enableIRQ() {
  PIC::enable(_irq_number);
}

void IRQHandler::disableIRQ() {
  PIC::disable(_irq_number);
}

