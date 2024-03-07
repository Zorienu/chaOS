#include <stdint.h>
#include <kernel/syscalls/syscalls.h>
#include <syscallNumbers.h>
#include <malloc.h>


int32_t syscallTest(SyscallRegisters regs) {
  return 10 + 20;
}

int32_t syscallMalloc(SyscallRegisters regs) {
  int32_t size = regs.ebx;

  void *ptr = mallocNextBlock(size);

  asm volatile("mov %0, %%edx" : : "g"(ptr));

  return EXIT_SUCCESS;
}

int32_t syscallFree(SyscallRegisters regs) {
  void *ptr = (void *)regs.ebx;

  mallocFree(ptr);

  return EXIT_SUCCESS;
}

/*
 * Syscall table
 */
int32_t (*syscalls[10])(SyscallRegisters) = {
  [SYSCALL_TEST] = syscallTest,
  [SYSCALL_MALLOC] = syscallMalloc,
  [SYSCALL_FREE] = syscallFree,
};

__attribute__ ((naked)) void syscallDispatcher(IntFrame32 *frame) {
  asm volatile (".intel_syntax noprefix\n"

                ".equ MAX_SYSCALLS, 10\n"     // Have to define again, inline asm does not see the #define

                "cmp eax, MAX_SYSCALLS-1\n"   // syscalls table is 0-based
                "ja invalid_syscall\n"        // invalid syscall number, skip and return

                "push eax\n"
                "push gs\n"
                "push fs\n"
                "push es\n"
                "push ds\n"
                "push ebp\n"
                "push edi\n"
                "push esi\n"
                "push edx\n"
                "push ecx\n"
                "push ebx\n"
                "push esp\n"
                "call [syscalls + eax * 4]\n"
                "add esp, 4\n"    // Do not overwrite esp
                "pop ebx\n"
                "pop ecx\n"

                // Do not overwrite EDX, as some syscalls e.g. malloc() will need it
                "add esp, 4\n"    

                "pop esi\n"
                "pop edi\n"
                "pop ebp\n"
                "pop ds\n"
                "pop es\n"
                "pop fs\n"
                "pop gs\n"
                "add esp, 4\n"    // Save eax value in case; don't overwrite it
                "iretd\n"         // Need interrupt return here! iret, NOT ret

                "invalid_syscall:\n"
                "mov eax, -1\n"   // Error will be -1
                "iretd\n"

                ".att_syntax");
} 
