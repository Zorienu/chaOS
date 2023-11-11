#include <stdint.h>
#include "../syscalls/syscallNumbers.h"

void *malloc(int32_t size) {
  void *ptr = 0x0;

  asm volatile("int $0x80" : "=d"(ptr) : "a"(SYSCALL_MALLOC), "b"(size));

  return ptr;
}
