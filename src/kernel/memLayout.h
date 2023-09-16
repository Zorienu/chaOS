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
