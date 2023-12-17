#include "../../include/mem/virtualMem.h"
#include "../../include/c/stdio.h"
#include "../../include/c/string.h"
#include <stddef.h>

static constexpr size_t INITIAL_KMALLOC_MEMORY_SIZE = 2 * MB;

__attribute__((section(".kernel_heap"))) static uint8_t initialKmallocMemory[INITIAL_KMALLOC_MEMORY_SIZE];

typedef struct KmallocBlock {
    uint32_t size;
    bool free;
    struct KmallocBlock *next;
} KmallocBlockType;

struct KmallocBlock *kmallocHead;

void pritnfKmallocInformation() {
  printf("\nkmalloc: %ld", INITIAL_KMALLOC_MEMORY_SIZE);
  printf("\nkmalloc address: %lx", initialKmallocMemory);
  printf("\nKmallocBlock sizeof: %d", sizeof(KmallocBlock));
}

void kmallocSplitBlock(size_t size, struct KmallocBlock *block) {
    struct KmallocBlock *newBlock = (struct KmallocBlock *)((uintptr_t)block + sizeof(KmallocBlock) + size);

    newBlock->size = block->size - size - sizeof(KmallocBlock);
    newBlock->free = true;
    newBlock->next = NULL;

    block->next = newBlock;
    block->free = false;
    block->size = size;
}

void kmallocInit() {
    memset(initialKmallocMemory, 0, INITIAL_KMALLOC_MEMORY_SIZE);

    kmallocHead = (KmallocBlock *)initialKmallocMemory;
    kmallocHead->size = INITIAL_KMALLOC_MEMORY_SIZE - sizeof(KmallocBlockType);
    kmallocHead->next = NULL;
    kmallocHead->free = true;
}

void *kmalloc(size_t size) {
    struct KmallocBlock *temp = kmallocHead;
    while (temp && (temp->size < size || !temp->free)) temp = temp->next;

    if (!temp) return NULL;

    if (temp->size == size) 
        temp->free = false;
    else if (temp->size > size + sizeof(KmallocBlock)) 
        kmallocSplitBlock(size, temp);

    
    return (void *)((uintptr_t)temp + sizeof(KmallocBlock));
}

void *kfree(void *ptr) {}

void *operator new(size_t size) {
    void *result = kmalloc(size);
    printf("\nCalling kmalloc with size: %d - %lx", size, result);
    return result;
}
 
void *operator new[](size_t size) {
    return kmalloc(size);
}
 
void operator delete(void *p) {
    return kfree(p);
}
 
void operator delete[](void *p) {
    return kfree(p);
}

void operator delete(void* ptr, size_t) {
    return kfree(ptr);
}

void operator delete[](void* ptr, size_t) {
    return kfree(ptr);
}

