#include <stdint.h>

#define MAX_IDT_ENTRIES 256

/*
 * P = present
 * DPL = ring
 * Type
 */
#define TRAP_GATE_FLAGS     0x8F    // P = 1, DPL = 00, S = 0, Type = 1111 (32bit trap gate)
#define INT_GATE_FLAGS      0x8E    // P = 1, DPL = 00, S = 0, Type = 1110 (32bit interrupt gate)
#define INT_GATE_USER_FLAGS 0xEE    // P = 1, DPL = 11, S = 0, Type = 1110 (32bit interrupt gate, called from PL 3)

/*
 * IDT (Interrupt Descriptor Table) entry (8 bytes)
 */
typedef struct {
  uint16_t isrAddressLow;  // Lower 16 bits of ISR (Interrupt Service Routine) address 
  uint16_t kernelCS;       // Code segment for this ISR (Interrupt Service Routine)
  uint8_t  reserved;       // Set to 0, reserved by Intel
  uint8_t  attributes;     // Type and attributes, flags
  uint16_t isrAddressHigh; // Upper 16 bits of ISR (Interrupt Service Routine) address
} __attribute__ ((packed)) IDTEntry32;

/*
 * IDTR (Interrupt Descriptor Table Register) layout
 */
typedef struct {
  uint16_t limit; 
  uint32_t base;
} __attribute__ ((packed)) IDTR32;

/*
 * Interrupt "frame" to pass to interrupt handlers/ISRs
 */
typedef struct {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t sp;
    uint32_t ss;
} __attribute__ ((packed)) IntFrame32;

/*
 * Set the given entry number within the IDT to use the given ISR
 */
void setIDTDescriptor(uint8_t entryNumber, void *isr, uint8_t flags);

/*
 * Initialize the IDT by loading its register (using lidt)
 */
void initIDT(void);
