#include "./syscallWrappers.h"
#include <stdint.h>

int32_t syscallTestWrapper() {
  int32_t result = -1;

  asm volatile("int $0x80" : "=a"(result) : "a"(0));

  return result;
}
