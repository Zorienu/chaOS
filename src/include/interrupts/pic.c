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

__attribute__ ((interrupt)) void keyboardIRQ1Handler(IntFrame32 *frame) {
  uint8_t key = inb(0x60);

  // Scancode set 1 -> Ascii lookup table
  const uint8_t *scancode_to_ascii = "\x00\x1B" "1234567890-=" "\x08"
  "\x00" "qwertyuiop[]" "\x0D\x1D" "asdfghjkl;'`" "\x00" "\\"
  "zxcvbnm,./" "\x00\x00\x00" " ";

  // printf("\nkey: %d %c", key, scancode_to_ascii[key]);

  if (scancode_to_ascii[key] == '1') cls();

  if (key == 28) printf("\n");
  else if (key < 100) printf("%c", scancode_to_ascii[key]);

  sendPICEndOfInterrupt(PIC_IRQ_KEYBOARD);
}
