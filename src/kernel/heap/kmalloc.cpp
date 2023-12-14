#include "../../include/mem/virtualMem.h"
#include "../../include/c/stdio.h"
#include <stddef.h>

static constexpr size_t INITIAL_KMALLOC_MEMORY_SIZE = 2 * MB;

__attribute__((section(".kernel_heap"))) static uint8_t initialKmallocMemory[INITIAL_KMALLOC_MEMORY_SIZE];

void printKmallocInitialMemory() {
  printf("\nkmalloc: %ld", INITIAL_KMALLOC_MEMORY_SIZE);
}

void *kmalloc(size_t size) {
  return 0x0;
}

void *kfree(void *ptr) {}


void *operator new(size_t size) {
    return kmalloc(size);
}
 
void *operator new[](size_t size) {
    return kmalloc(size);
}
 
void operator delete(void *p) {
    kfree(p);
}
 
void operator delete[](void *p) {
    kfree(p);
}
