#include <stdint.h>

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
