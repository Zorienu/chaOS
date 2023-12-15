#include <stddef.h>

/*
 * Print kmalloc information for debugging
 */
void pritnfKmallocInformation();

/*
 * Initialize kmalloc, setting all reserved memory to 0 and initializing the head of the singly linked list
 */
void kmallocInit();

/*
 * Malloc implementation for the kernel
 */
void *kmalloc(size_t size);
