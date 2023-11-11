#pragma once
#include <stdint.h>

#define EXIT_SUCCESS 0

/*
 * Registers pushed onto stack when a syscall function is called
 * from the syscall dispatcher
 */
typedef struct {
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t eax;
} __attribute__ ((packed)) SyscallRegisters;


/*
 * Syscall dispatcher, called when a syscall is invoked
 * "naked" means: do not add function epilogue/prologue, and only allow inline asm
 *
 * Push everything we want to save
 * Call the syscall by offseting into the syscall table by using the value in eax
 * Pop everything back
 * Return using "iret" since it's an interrupt (software interrupt)
 *
 * Already on stack: SS, SP, FLAGS, CS, IP
 * Need to push: AX, GS, FS, ES, DS, BP, DI, SI, DX, CX, BX
 */
__attribute__ ((naked)) void syscallDispatcher(void); 
