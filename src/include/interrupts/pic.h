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
