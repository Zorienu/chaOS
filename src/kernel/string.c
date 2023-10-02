/*
 * Get the size in bytes of the given string
 *
 * string: the string to calculate the length
 *
 * returns: the size of the string
 */
#include <stddef.h>
#include <stdint.h>
unsigned long strlen (char *string) {
  int result = 0;

  while(*string++) result++; 

  return result;
}

void memcpy(void *_destination, void *_source, size_t count) {
  uint8_t *destination = (uint8_t *)_destination;
  uint8_t *source = _source;

  while (count--) 
    *destination++ = *source++;
}
 
void memset(void *_destination, uint8_t value, size_t count) {
  uint8_t *destination = (uint8_t *)_destination;

  while (count--)
    *destination++ = value;
}
