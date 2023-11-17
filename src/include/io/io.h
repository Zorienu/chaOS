#include <stdint.h>

/* Ports */
#define DISK_PORT_BASE 0x1F0

/* Commands */
#define DISK_READ_CMD 0x20

static inline uint8_t inb(uint16_t port) {
  uint8_t data;

  asm volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

static inline void outb(uint16_t port, uint8_t data) {
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

/*
 * Performs the input operation from the given port to the given address
 * 'count' times.
 *
 * 'rep insl' is a repeated input operation and it reads a 32-value in each iteration from
 * the I/O port specified in 'dx' register and stores it in memory pointed by the 'edi' register.
 * The 'ecx' register determines how many times this operation is repeated (controlled by 'count' in the C function)
 *
 * So if we need to read 8 bytes (64 bits), the 'count' parameter needs to be '2' (2 * 32 bits = 64 bits)
 * 'cld' clears the direction flag (DF) in the EFLAGS register. This ensures that string operations
 * line 'insl' move data from lower memory addresses to higher ones.
 */
static inline void insl(int port, void *address, int count) {
  asm volatile("cld; rep insl" :
               "=D" (address), "=c" (count) :
               "d" (port), "0" (address), "1" (count) :
               "memory", "cc");
}

/*
 * Fill 'count' words with 'data' starting from 'address'
 */
static inline void stosb(void *address, int data, int count) {
  asm volatile("cld; rep stosb" :
               "=D" (address), "=c" (count) :
               "0" (address), "1" (count), "a" (data) :
               "memory", "cc");
}

/*
 * Wait 1 I/O cycle for I/O operations to complete
 */
static inline void ioWait(void) {
  // Port 0x80 is used for checkpoints during POST
  //  Linux uses this, so we "should" be OK to use it also
  asm volatile("outb %%al, $0x80" : : "a"(0) );
}
