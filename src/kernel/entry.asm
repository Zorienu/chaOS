; Entry for the kernel
; This will setup paging and jump to src/kernel/main.c -> OSStart 
section .text

extern OSStart            ; Defined in main.c

global entry
entry:
  jmp OSStart

