#pragma once
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

/*
 * Free implementation for the kernel
 */
void kfree(void *ptr);

void *operator new(size_t size, void *ptr);
