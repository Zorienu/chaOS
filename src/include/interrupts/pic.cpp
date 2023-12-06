#include <stdint.h>
#include "pic.h"
#include "../io/io.h"
#include "../c/stdio.h"

void initializePIC() {
  // Get and save current mask
  uint8_t masterPICMask = inb(PIC_MASTER_IMR_REGISTER_PORT);
  uint8_t slavePICMask = inb(PIC_SLAVE_IMR_REGISTER_PORT);

  // Set bit 0 -> we will send ICW4 later
  // Set bit 4 -> initialize PIC
  // Resulting in 0x11
  uint8_t icw1 = ICW1_PIC_RECEIVES_ICW4     |
                 ICW1_SLAVE_PIC_PRESENT     |
                 ICW1_CALL_ADDRESS_INTERVAL |
                 ICW1_PIC_TRIGGER_MODE      |
                 ICW1_PIC_INITIALIZATION;

  // Send ICW 1 to the master PIC command register
  outb(PIC_MASTER_COMMAND_REGISTER_PORT, icw1);
  ioWait();
  // Send ICW 1 to the slave PIC command register
  outb(PIC_SLAVE_COMMAND_REGISTER_PORT, icw1);
  ioWait();

  // Send ICW 2 to the master PIC data register:
  // IRQ 0 is now mapped to interrupt 0x20
  outb(PIC_MASTER_DATA_REGISTER_PORT, PIC_IRQ_0_IDT_ENTRY);
  ioWait();
  // Send ICW 2 to the slave PIC data register:
  // IRQ 8 is now mapped to interrupt 0x28
  outb(PIC_SLAVE_DATA_REGISTER_PORT, PIC_IRQ_8_IDT_ENTRY);
  ioWait();

  // Send ICW 3 to master PIC data register:
  outb(PIC_MASTER_DATA_REGISTER_PORT, PIC_MASTER_IRQ_BIT_CONNECTED_TO_SLAVE);
  ioWait();
  // Send ICW 3 to slave PIC data register:
  outb(PIC_SLAVE_DATA_REGISTER_PORT, PIC_SLAVE_IRQ_BIT_CONNECTED_TO_MASTER);
  ioWait();

  // Send ICW 4 to master PIC data register:
  outb(PIC_MASTER_DATA_REGISTER_PORT, PIC_80X86_MODE);
  ioWait();
  // Send ICW 4 to slave PIC data register:
  outb(PIC_SLAVE_DATA_REGISTER_PORT, PIC_80X86_MODE);
  ioWait();

  // Restore previously saved masks
  outb(PIC_MASTER_DATA_REGISTER_PORT, masterPICMask);
  outb(PIC_SLAVE_DATA_REGISTER_PORT, slavePICMask);
}

void sendPICEndOfInterrupt(uint8_t irq) {
  if (irq >= 8) outb(PIC_SLAVE_COMMAND_REGISTER_PORT, PIC_END_OF_INTERRUPT_COMMAND);
 
  outb(PIC_MASTER_COMMAND_REGISTER_PORT, PIC_END_OF_INTERRUPT_COMMAND);
}

void disablePIC(void) {
  // Disable all IRQ lines in master PIC
  outb(PIC_SLAVE_IMR_REGISTER_PORT, 0xFF);
  // Disable all IRQ lines in slave PIC
  outb(PIC_MASTER_IMR_REGISTER_PORT, 0xFF);
}

void enableIRQ(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) port = PIC_MASTER_DATA_REGISTER_PORT;
    else {
        irq -= 8;
        port = PIC_SLAVE_DATA_REGISTER_PORT;
    }

    // Get current IMR value, clear the IRQ bit to unmask it, then write new value to IMR
    // NOTE: Clearing IRQ2 will enable the 2nd PIC to raise IRQs due to 2nd PIC being mapped to IRQ2 in PIC1
    value = inb(port) & ~(1 << irq);
    outb(port, value);
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
  outb(PIT_CONTROL_WORD_PORT, cw);

  uint8_t msb = counter >> 8;
  uint8_t lsb = counter & 0xFF;


  outb(PIT_COUNTER_0_PORT + channel, lsb);
  outb(PIT_COUNTER_0_PORT + channel, msb);

  asm volatile("sti");
}

__attribute__ ((interrupt)) void keyboardIRQ1Handler(IntFrame32 *frame) {
  uint8_t key = inb(0x60);

  // Scancode set 1 -> Ascii lookup table
  const char scancode_to_ascii[] = "\x00\x1B" "1234567890-=" "\x08"
  "\x00" "qwertyuiop[]" "\x0D\x1D" "asdfghjkl;'`" "\x00" "\\"
  "zxcvbnm,./" "\x00\x00\x00" " ";

  // printf("\nkey: %d %c", key, scancode_to_ascii[key]);

  if (scancode_to_ascii[key] == '1') cls();

  if (key == 28) printf("\n");
  else if (key < 100) printf("%c", scancode_to_ascii[key]);

  sendPICEndOfInterrupt(PIC_IRQ_KEYBOARD);
}

volatile uint16_t testt = 0;
volatile uint32_t test2 = 0;

__attribute__ ((interrupt)) void pitIRQ0Handler(IntFrame32 *frame) {
  if (testt++ == 2000)
  {
    printf("\nTimer: %ld", test2++);
    testt=0;
  }

  sendPICEndOfInterrupt(PIC_IRQ_TIMER);
}