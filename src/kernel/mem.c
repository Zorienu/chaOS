#include <stddef.h>
#include <stdint.h>

void memcpy(void *_destination, void *_source, size_t count) {
  uint8_t *destination = _destination;
  uint8_t *source = _source;

  while (count--) 
    *destination++ = *source++;
}
 
void memset(void *_destination, uint8_t value, size_t count) {
  uint8_t *destination = _destination;

  while (count--)
    *destination++ = value;
}
