#include "Assertions.h"

void assertionFailed(const char* msg, const char* file, unsigned line, const char* func) {
    asm volatile("cli");
    kprintf("ASSERTION FAILED: %s\n%s:%u in %s\n", msg, file, line, func);
    asm volatile("hlt");
    for (;;)
        ;
}
