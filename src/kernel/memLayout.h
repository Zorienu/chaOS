#include <stdint.h>

#define KERNEL_BASE 0x80000000 // 2 GiB
#define IO_SPACE    0x100000 
#define PHYSICAL_STOP 0xE000000 // Total physical memory - 224 MiB

/*
 * Convert the given virtual address to physical address
 */
#define V2P_WO(x) ((x) - KERNEL_BASE) 

/*
 * Convert the given virtual address to physical address
 */
void *V2P(void *va);

/*
 * Convert the given physical address to virtual address
 */
void *P2V(void *pa);

/* 
 * Align up the given address
 * @param {uint32_t} address - The address we want to align up
 * @example 0x1000 -> 0x1000
 * @example 0x1001 -> 0x2000
 * @example 0x1FFF -> 0x2000
 * @example 0x2000 -> 0x2000
 */
uint32_t ALIGN_ADDRESS_UP(uint32_t address);


/* 
 * Align down the given address
 * @param {uint32_t} address - The address we want to align down
 * @example 0x1000 -> 0x1000
 * @example 0x1001 -> 0x1000
 * @example 0x1FFF -> 0x1000
 * @example 0x2000 -> 0x2000
 */
uint32_t ALIGN_ADDRESS_DOWN(uint32_t address);
