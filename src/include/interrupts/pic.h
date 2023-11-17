/*
 * The following devices used the PIC 1 (master PIC) to generate interrupts
 * Remember that line 2 is connected to the PIC 2 (slave PIC)
 * Also lower the line number -> higher the priority of the request
 */
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
#define PIC_MASTER_COMMAND_AND_STATUS_REGISTER_PORT 0x20
#define PIC_SLAVE_COMMAND_AND_STATUS_REGISTER_PORT  0xA0
// IMR: Interrupt Mask Register -> write-only
// Data register                -> read-only
#define PIC_MASTER_IMR_AND_DATA_REGISTER_PORT       0x21
#define PIC_SLAVE_IMR_AND_DATA_REGISTER_PORT        0xA1

/*
 * ICW 1 configuration bits
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
 * Initialize the PIC (Programmable Interrupt Controller) 
 * by sending the appropiate ICW (Initialization Control Words)
 */
void initializePIC();
