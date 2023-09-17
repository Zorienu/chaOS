#include "memLayout.h"
#include "mmu.h"
#include <stdint.h>

void *V2P(void *va) {
  return va - KERNEL_BASE;
}

void *P2V(void *pa) {
  return pa + KERNEL_BASE;
}

uint32_t ALIGN_ADDRESS_UP(uint32_t address) {
  return (address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

uint32_t ALIGN_ADDRESS_DOWN(uint32_t address) {
  return address & ~(PAGE_SIZE - 1);
}
