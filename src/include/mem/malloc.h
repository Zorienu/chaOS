#include <stdint.h>
#include <stdbool.h>

typedef struct MallocBlock { 
  uint32_t size;                // Size in bytes of the block
  bool free;                    // Whether the block is free or not
  struct MallocBlock *next;     // Next block
} MallocBlockType;

/*
 * Allocate the next free block of memory
 */
void *mallocNextBlock(uint32_t size);

/*
 * Free the block pointed by the given address
 */
void mallocFree(void *ptr);
