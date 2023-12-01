#pragma once
#include <stddef.h>
#include <stdint.h>

/*
 * Get the size in bytes of the given string
 *
 * string: the string to calculate the length
 *
 * returns: the size of the string
 */
unsigned long strlen (char *string);

/*
 * Set the given number of bytes from source to destination
 */
void memcpy(void *_destination, void *_source, size_t count);

/*
 * Set the given number of bytes to the given value starting from the given address
 */
void memset(void *_destination, uint8_t value, size_t count);
