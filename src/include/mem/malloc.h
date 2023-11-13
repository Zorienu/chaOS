#include <stdint.h>
#include <stdbool.h>

/*
 * Contains information about a malloc-ed block of memory
 */
typedef struct MallocBlock { 
  uint32_t size;                // Size in bytes of the block
  bool free;                    // Whether the block is free or not
  struct MallocBlock *next;     // Next block
} MallocBlockType;

/*
 * Allocate the next free block of memory
 * Allocate more physical pages if needed
 */
void *mallocNextBlock(uint32_t size);

/*
 * Free the block pointed by the given address
 * Then merge consecutives free blocks
 */
void mallocFree(void *ptr);
