#include <stdint.h>

#define MAX_IDT_ENTRIES 256

/*
 * IDT (Interrupt Descriptor Table) entry
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

void setIDTDescriptor(uint8_t entryNumber, void *isr, uint8_t flags);

void initIDT(void);
