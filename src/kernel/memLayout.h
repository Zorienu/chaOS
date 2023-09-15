#define KERNEL_BASE 0x80000000 // 2 GiB
#define IO_SPACE    0x100000 

/*
 * Convert the given virtual address to physical address
 */
#define V2P_WO(x) ((x) - KERNBASE) 
