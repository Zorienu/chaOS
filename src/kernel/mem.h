#include <stddef.h>
#include <stdint.h>

/*
 * Set the given number of bytes from source to destination
 */
void memcpy(void *_destination, void *_source, size_t count);

/*
 * Set the given number of bytes to the given value starting from the given address
 */
void memset(void *_destination, uint8_t value, size_t count);

