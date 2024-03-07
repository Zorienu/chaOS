#include <kernel/heap/kmalloc.h>
#include <virtualMem.h>
#include <string.h>
#include <stddef.h>
#include <kernel/utils/kprintf.h>

static constexpr size_t INITIAL_KMALLOC_MEMORY_SIZE = 2 * MB;

static uint8_t initialKmallocMemory[INITIAL_KMALLOC_MEMORY_SIZE];

typedef struct KmallocBlock {
    uint32_t size;
    bool free;
    struct KmallocBlock *next;
} KmallocBlockType;

struct KmallocBlock *kmallocHead;

void pritnfKmallocInformation() {
  kprintf("\nkmalloc: %ld", INITIAL_KMALLOC_MEMORY_SIZE);
  kprintf("\nkmalloc address: %lx", initialKmallocMemory);
  kprintf("\nKmallocBlock sizeof: %d", sizeof(KmallocBlock));
  kprintf("\n=== Kmalloc blocks ===");

  for (struct KmallocBlock *temp = kmallocHead; temp; temp = temp->next) 
      kprintf("\nAddress: %lx - Size: %d - Free: %d - Next: %lx", temp, temp->size, temp->free, temp->next);

}

/*
 * Split the given kmalloc block into two, leaving the first with the requested size and the other with 
 * the remaining size
 */
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

/*
 * Iterate over the kmalloc singly linked list and join the consecutive free blocks
 */
void kmallocJoinFreeBlocks() {
    struct KmallocBlock *temp = kmallocHead;

    while(temp) {
        if (temp->free && temp->next && temp->next->free) {
            temp->size += temp->next->size + sizeof(KmallocBlock);
            temp->next = temp->next->next;
        }

        temp = temp->next;
    }
}

void kfree(void *ptr) {
    struct KmallocBlock *temp = kmallocHead;

    for (; temp; temp = temp->next) {
        kprintf("\nkfree: temp: %lx - %lx", temp, temp + 1);
        if (temp + 1 == ptr) temp->free = true;
        kmallocJoinFreeBlocks();
    }
}

void *operator new(size_t size) {
    void *result = kmalloc(size);
    return result;
}
 
void *operator new[](size_t size) {
    return kmalloc(size);
}

void *operator new(size_t size, void *ptr) {
    return ptr;
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

