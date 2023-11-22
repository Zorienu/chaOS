/*
 * The following devices used the PIC 1 (master PIC) to generate interrupts
 * Remember that line 2 is connected to the PIC 2 (slave PIC)
 * Also lower the line number -> higher the priority of the request
 */
#include <stdint.h>
#define PIC_IRQ_TIMER      0
#define PIC_IRQ_KEYBOARD   1
#define PIC_IRQ_SERIAL_2   3
#define PIC_IRQ_SERIAL_1   4
#define PIC_IRQ_PARALLEL_2 5
#define PIC_IRQ_DISKETTE   6
#define PIC_IRQ_PARALLEL_1 7

/*
 * The following devices used the PIC 2 (slave PIC) to generate interrupts
 */
#define PIC_IRQ_CMOS_TIMER 0
#define PIC_IRQ_CGARETRACE 1
#define PIC_IRQ_AUXILIARY  4
#define PIC_IRQ_FPU        5
#define PIC_IRQ_HDC        6

/*
 * PIC 8259A software port map
 */
// Command register -> write-only
// Status register  -> read-only
#define PIC_MASTER_COMMAND_REGISTER_PORT 0x20
#define PIC_MASTER_STATUS_REGISTER_PORT  0x20
#define PIC_SLAVE_COMMAND_REGISTER_PORT  0xA0
#define PIC_SLAVE_STATUS_REGISTER_PORT   0xA0
// IMR: Interrupt Mask Register -> read-only
// Data register                -> write-only
#define PIC_MASTER_IMR_REGISTER_PORT     0x21 // Interrupt Mask Register
#define PIC_MASTER_DATA_REGISTER_PORT    0x21
#define PIC_SLAVE_IMR_REGISTER_PORT      0xA1 // Interrupt Mask Register
#define PIC_SLAVE_DATA_REGISTER_PORT     0xA1

/*
 * ICW 1 configuration bits
 * Primary control word used for Initialize the PIC
 */
// Bit 0 - We well send ICW4 to the PIC?
#define ICW1_PIC_RECEIVES_ICW4 1
// Bit 1 - Slave PIC is present, send clear bit (we'll have to send ICW3)
#define ICW1_SLAVE_PIC_PRESENT 0
// Bit 2 - Ignored in x86, send clear bit
#define ICW1_CALL_ADDRESS_INTERVAL 0
// Bit 3 - set: operate in Level Triggered Mode, clear: operate in Edge Triggered Mode (we we'll use ETM)
#define ICW1_PIC_TRIGGER_MODE 0
// Bit 4 - Initialization bit, set to 1
#define ICW1_PIC_INITIALIZATION 1 << 4
// Bits 5-7 are set to 0

/*
 * ICW 2 configuration bits
 * Used for mapping the base address of the IDT that the PIC will use
 */
// 0x20 (32 in decimal) will be the entry within the IDT for handling the PIT (Programmable Interrupt Timer) interrupts
// 0x20 because we skip the first 32 interrupts (0x0 - 0x1F)
// which correspond to software interrupts like divide by 0
// 0x20 - 0x27 for IRQs 0 - 7
#define PIC_IRQ_0_IDT_ENTRY 0x20
// 0x28 (40 in decimal) will be the entry within the IDT for handing the RTC (Real Time Clock) interrupts
// 0x28 - 0x35 for IRQs 8 - 15
#define PIC_IRQ_8_IDT_ENTRY 0x28

/*
 * ICW 3 configuration bits
 * Used to let the PICs (master and slave) know what IRQ lines to use when communicating with each other
 * We must send ICW 3 whenever we enable cascading with ICW 1 and
 * let each PIC know about each other and how they are connected
 * We do this by sending the ICW 3 to both PICs containing which IRQ line to use 
 * for both the master and associated PICs
 * The 80x86 architecture uses IRQ line 2 to connect the master PIC to the slave PIC.
 * In the ICW 3 for the master PIC, each bit represents an IRQ (1 << 0 -> IRQ 0, 1 << 7 -> IRQ 7)
 */
#define PIC_MASTER_IRQ_BIT_CONNECTED_TO_SLAVE 1 << 2 // IRQ 2 connected to bit 2 -> 0B100 -> set bit 2 to 1
// For the slave PIC, we must send the IRQ line to be used in binary notation, since we only have 3 bits for this
// and the other bits (4 - 7) are reserved
#define PIC_SLAVE_IRQ_BIT_CONNECTED_TO_MASTER 2 // IRQ 2 in bynary notation -> 0B010

/*
 * ICW 4 configuration bits
 * We set the first bit in ICW 1 to 1, so, the PICs expect we send ICW 4
 * bits 5-7 always 0
 * We just set the PICs to use 80x86 mode by setting the bit 0
 */
#define PIC_80X86_MODE 1

#define PIC_END_OF_INTERRUPT_COMMAND 0x20 // Set bit 5 in the ICW 2

/*
 * Initialize the PIC (Programmable Interrupt Controller) 
 * by sending the appropiate ICW (Initialization Control Words)
 */
void initializePIC();

/* 
 * Tell to the PICs (master and slave) that we finished handling the interrupt
 * ensuring all hardware interrupts are enabled now
 */
void sendPICEndOfInterrupt(uint8_t irq);

/*
 * Disable all IRQ lines in master and slave PICs by setting all the bits in the IMR (Interrupt Mask Register)
 */
void disablePIC(void);
