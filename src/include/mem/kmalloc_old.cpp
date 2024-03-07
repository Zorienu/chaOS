#include <stddef.h>
#include <stdlib.h>

/*
 * TODO: compile and link only for user-space, 
 * for kernel there is the kmalloc and kfree implementation in kernel/heap/kmalloc.cpp
 */
/*
void *operator new(size_t size)
{
    return malloc(size);
}
 
void *operator new[](size_t size)
{
    return malloc(size);
}
 
void operator delete(void *p)
{
    free(p);
}
 
void operator delete[](void *p)
{
    free(p);
}
*/
