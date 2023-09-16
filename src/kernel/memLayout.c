#include "memLayout.h"

void *V2P(void *va) {
  return va - KERNEL_BASE;
}

void *P2V(void *pa) {
  return pa + KERNEL_BASE;
}
