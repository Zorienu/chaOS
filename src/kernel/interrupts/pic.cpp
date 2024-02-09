#include <stdint.h>
#include "pic.h"
#include "../../include/io/io.h"
#include "../utils/kprintf.h"

namespace PIC {

void initializePIC() {
  // Get and save current mask
  uint8_t masterPICMask = IO::inb(PIC_MASTER_IMR_REGISTER_PORT);
  uint8_t slavePICMask = IO::inb(PIC_SLAVE_IMR_REGISTER_PORT);

  // Set bit 0 -> we will send ICW4 later
  // Set bit 4 -> initialize PIC
  // Resulting in 0x11
  uint8_t icw1 = ICW1_PIC_RECEIVES_ICW4     |
                 ICW1_SLAVE_PIC_PRESENT     |
                 ICW1_CALL_ADDRESS_INTERVAL |
                 ICW1_PIC_TRIGGER_MODE      |
                 ICW1_PIC_INITIALIZATION;

  // Send ICW 1 to the master PIC command register
  IO::outb(PIC_MASTER_COMMAND_REGISTER_PORT, icw1);
  IO::wait();
  // Send ICW 1 to the slave PIC command register
  IO::outb(PIC_SLAVE_COMMAND_REGISTER_PORT, icw1);
  IO::wait();

  // Send ICW 2 to the master PIC data register:
  // IRQ 0 is now mapped to interrupt 0x20
  IO::outb(PIC_MASTER_DATA_REGISTER_PORT, PIC_IRQ_0_IDT_ENTRY);
  IO::wait();
  // Send ICW 2 to the slave PIC data register:
  // IRQ 8 is now mapped to interrupt 0x28
  IO::outb(PIC_SLAVE_DATA_REGISTER_PORT, PIC_IRQ_8_IDT_ENTRY);
  IO::wait();

  // Send ICW 3 to master PIC data register:
  IO::outb(PIC_MASTER_DATA_REGISTER_PORT, PIC_MASTER_IRQ_BIT_CONNECTED_TO_SLAVE);
  IO::wait();
  // Send ICW 3 to slave PIC data register:
  IO::outb(PIC_SLAVE_DATA_REGISTER_PORT, PIC_SLAVE_IRQ_BIT_CONNECTED_TO_MASTER);
  IO::wait();

  // Send ICW 4 to master PIC data register:
  IO::outb(PIC_MASTER_DATA_REGISTER_PORT, PIC_80X86_MODE);
  IO::wait();
  // Send ICW 4 to slave PIC data register:
  IO::outb(PIC_SLAVE_DATA_REGISTER_PORT, PIC_80X86_MODE);
  IO::wait();

  // Restore previously saved masks
  IO::outb(PIC_MASTER_DATA_REGISTER_PORT, masterPICMask);
  IO::outb(PIC_SLAVE_DATA_REGISTER_PORT, slavePICMask);
}

void sendEOI(uint8_t irq) {
  if (irq >= 8) IO::outb(PIC_SLAVE_COMMAND_REGISTER_PORT, PIC_END_OF_INTERRUPT_COMMAND);
 
  IO::outb(PIC_MASTER_COMMAND_REGISTER_PORT, PIC_END_OF_INTERRUPT_COMMAND);
}

void disableAll(void) {
  // Disable all IRQ lines in master PIC
  IO::outb(PIC_SLAVE_IMR_REGISTER_PORT, 0xFF);
  // Disable all IRQ lines in slave PIC
  IO::outb(PIC_MASTER_IMR_REGISTER_PORT, 0xFF);
}

void enable(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) port = PIC_MASTER_DATA_REGISTER_PORT;
    else {
        irq -= 8;
        port = PIC_SLAVE_DATA_REGISTER_PORT;
    }

    // Get current IMR value, clear the IRQ bit to unmask it, then write new value to IMR
    // NOTE: Clearing IRQ2 will enable the 2nd PIC to raise IRQs due to 2nd PIC being mapped to IRQ2 in PIC1
    value = IO::inb(port) & ~(1 << irq);
    IO::outb(port, value);
}

void disable(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) port = PIC_MASTER_DATA_REGISTER_PORT;
    else {
        irq -= 8;
        port = PIC_SLAVE_DATA_REGISTER_PORT;
    }

    // Get current IMR value, set the IRQ bit to mask it, then write new value to IMR
    // NOTE: Clearing IRQ2 will enable the 2nd PIC to raise IRQs due to 2nd PIC being mapped to IRQ2 in PIC1
    value = IO::inb(port) | (1 << irq);
    IO::outb(port, value);
}

void configurePIT(uint8_t channel, uint8_t operatingMode, uint16_t counter) {
  if (channel > 2) return;

  asm volatile("cli");

  /*
   * Configuration Word for the PIT
   * bit 0: Binary counter
   *   0: Binary <-
   *   1: Binary Coded Decimal (BCD)
   * bit 1-3: (M0, M1, M2) Operating mode
   *   000: Mode 0: Interrupt or Terminal Count
   *   001: Mode 1: Programmable one-shot
   *   010: Mode 2: Rate Generator
   *   011: Mode 3: Square Wave Generator
   *   100: Mode 4: Software Triggered Strobe
   *   101: Mode 5: Hardware Triggered Strobe
   *   110: Undefined; Don't use
   *   111: Undefined; Don't use
   * bit 4-5: (RL0, RL1) Read/Load mode. We are going to read or send data to a counter register
   *   00: Counter value is latched into an internal control register at the time of the I/O write operation.
   *   01: Read or Load Least Significant Byte (LSB) only
   *   10: Read or Load Most Significant Byte (MSB) only
   *   11: Read or Load LSB first then MSB <-
   * bit 6-7: (SC0, SC1) Select counter or channel (0-2)
   *   00: Counter 0
   *   01: Counter 1
   *   10: Counter 2
   *   11: Illegal value
   */
  uint8_t cw = channel << 6  | 3 << 4 | operatingMode << 1;
  IO::outb(PIT_CONTROL_WORD_PORT, cw);

  uint8_t msb = counter >> 8;
  uint8_t lsb = counter & 0xFF;


  IO::outb(PIT_COUNTER_0_PORT + channel, lsb);
  IO::outb(PIT_COUNTER_0_PORT + channel, msb);

  asm volatile("sti");
}

volatile uint16_t testt = 0;
volatile uint32_t test2 = 0;

__attribute__ ((interrupt)) void pitIRQ0Handler(IntFrame32 *frame) {
  if (testt++ == 100) // use 2000 with bochs
  {
    // kprintf("\nTimer: %ld", test2++);
    testt=0;
  }

  sendEOI(PIC_IRQ_TIMER);
}

uint16_t getISR() {
  IO::outb(PIC_MASTER_STATUS_REGISTER_PORT, 0x0b);
  IO::outb(PIC_SLAVE_COMMAND_REGISTER_PORT, 0x0b);
  uint8_t isr0 = IO::inb(PIC_MASTER_STATUS_REGISTER_PORT);
  uint8_t isr1 = IO::inb(PIC_SLAVE_STATUS_REGISTER_PORT);
  return (isr1 << 8) | isr0;
}

}
