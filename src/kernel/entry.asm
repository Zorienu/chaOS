; Entry for the kernel
; This will setup paging and jump to src/kernel/main.c -> OSStart 
section .text

extern entryPageDirectory ; Defined in mmu.h
extern OSStart            ; Defined in main.c

; TODO: avoid defining this again,
; use the values defined in memLayout.h and mmu.h
%define KERNEL_BASE 0xC0000000
%define CR0_PG      0x80000000
%define CR4_PSE     0x00000010

global entry
entry:
  ; Load the page directory into CR3
  ; mov eax, dword entryPageDirectory - 0x80000000
  ; mov eax, dword $(V2P_WO(entryPageDirectory))
  mov eax, dword entryPageDirectory - KERNEL_BASE
  mov cr3, eax

  ; Turning on PSE (Page Size Extension) for using 4MiB pages 
  ; by setting the fifth bit in the CR4 register
  mov eax, cr4
  or eax, CR4_PSE
  mov cr4, eax

  ; Enabling paging and write protection setting the CR0 register
  mov eax, cr0
  or eax, CR0_PG
  mov cr0, eax

  jmp OSStart

