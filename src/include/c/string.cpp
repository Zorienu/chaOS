#include <stddef.h>
#include <stdint.h>

/*
 * Get the size in bytes of the given string
 *
 * string: the string to calculate the length
 *
 * returns: the size of the string
 */
unsigned long strlen (char *string) {
  int result = 0;

  while(*string++) result++; 

  return result;
}

bool strcmp(char *value, const char* target) {
  int i = 0;

  while (value[i] || target[i]) {
    if (value[i] != target[i]) return false; 

    i++;
  }

  return true;
}

void memcpy(void *_destination, void *_source, size_t count) {
  uint8_t *destination = (uint8_t *)_destination;
  uint8_t *source = (uint8_t *)_source;

  while (count--) 
    *destination++ = *source++;
}
 
void memset(void *_destination, uint8_t value, size_t count) {
  uint8_t *destination = (uint8_t *)_destination;

  while (count--)
    *destination++ = value;
}
