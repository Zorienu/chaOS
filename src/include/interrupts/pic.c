#include "pic.h"
#include "../io/io.h"
#include <stdint.h>

void initializePIC() {
  // Set bit 0 -> we will send ICW4 later
  // Set bit 4 -> initialize PIC
  // Resulting in 0x11
  uint8_t icw1 = ICW1_PIC_RECEIVES_ICW4     |
                 ICW1_SLAVE_PIC_PRESENT     |
                 ICW1_CALL_ADDRESS_INTERVAL |
                 ICW1_PIC_TRIGGER_MODE      |
                 ICW1_PIC_INITIALIZATION;

  // Send ICW 1 to the master PIC command register
  outb(PIC_MASTER_COMMAND_AND_STATUS_REGISTER_PORT, icw1);
  ioWait();
  outb(PIC_SLAVE_COMMAND_AND_STATUS_REGISTER_PORT, icw1);
  ioWait();

  // Send ICW 2 to the master PIC command register:
  // IRQ 0 is not mapped to interrupt 0x20
  outb(PIC_MASTER_COMMAND_AND_STATUS_REGISTER_PORT, PIC_IRQ_0_IDT_ENTRY);
  ioWait();
  // IRQ 8 is not mapped to interrupt 0x28
  outb(PIC_SLAVE_COMMAND_AND_STATUS_REGISTER_PORT, PIC_IRQ_8_IDT_ENTRY);
  ioWait();

}
