#include <stdint.h>
#include "malloc.h"
#include "mem.h"
#include "virtualMem.h"
#include "../c/stdio.h"

MallocBlockType *mallocListHead = 0;
VirtualAddress mallocVirtualAddress = 0;
PhysicalAddress mallocPhysicalAddress = 0;
uint32_t totalMallocPages = 0;

void mallocInit(uint32_t size) {
  mallocPhysicalAddress = (PhysicalAddress)allocateBlock();
  mallocVirtualAddress = 0x300000;

  mapPage(mallocVirtualAddress, mallocPhysicalAddress);

  mallocListHead = (MallocBlockType *)mallocVirtualAddress;

  mallocListHead->size = PAGE_SIZE - sizeof(MallocBlockType);
  mallocListHead->free = true;
  mallocListHead->next = 0;

  totalMallocPages = 1;
}

void mallocSplit(MallocBlockType *temp, uint32_t size) {
  MallocBlockType *newBlock = (void *)temp + sizeof(MallocBlockType) + size;
  printf("\n-->Temp address: %lx", temp);
  printf("\n-->New block: %lx", newBlock);

  newBlock->free = true;
  newBlock->size = temp->size - size - sizeof(MallocBlockType);
  newBlock->next = 0;

  temp->next = newBlock;
  temp->free = false;
  temp->size = size;

  printf("\n-->Old block size: %d", temp->size);
  printf("\n-->New block size: %d", newBlock->size);
}

void *mallocNextBlock(uint32_t size) {
  // TODO: review if this is ok
  if (!mallocListHead) mallocInit(size);

  // Nothing to malloc
  if (!size) return 0;

  MallocBlockType *temp = mallocListHead;

  printf("\n-->Next block: %lx", temp->next);

  // Find next big enough free block
  while (((temp->size < size) || !temp->free) && temp->next) {
    printf("\nInside while: %lx", temp);
    temp = temp->next;
  }

  printf("\nFree block: %lx", temp);

  if (size == temp->size && temp->free) {
    temp->free = false;
  }
  else if (temp->size > size + sizeof(MallocBlockType)) {
    mallocSplit(temp, size);
    printf("\nAfter malloc split: temp.next: %lx", temp->next);
  } else {
    uint32_t neededAdditionalBytes = size + sizeof(MallocBlockType) - temp->size;
    uint32_t neededAdditionalPages = neededAdditionalBytes / PAGE_SIZE;

    if (neededAdditionalBytes % PAGE_SIZE) neededAdditionalPages++;

    while (neededAdditionalPages) {
      totalMallocPages++;
      neededAdditionalPages--;

      mapPage(mallocVirtualAddress + totalMallocPages * PAGE_SIZE, (PhysicalAddress)allocateBlock());

      temp->size += PAGE_SIZE;
    }

    mallocSplit(temp, size);
  }

  return (void *)temp + sizeof(MallocBlockType);
}
