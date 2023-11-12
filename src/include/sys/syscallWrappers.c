#include <stdint.h>
#include "./syscallWrappers.h"
#include "../syscalls/syscallNumbers.h"

int32_t syscallTestWrapper() {
  int32_t result = -1;

  asm volatile("int $0x80" : "=a"(result) : "a"(SYSCALL_TEST));

  return result;
}


