; x86 assembly language
; How a computer starts up?
; BIOS is copied from a ROM chip into RAM
; BIOS starts executing code
; - initialize hardware
; - runs some tests (POST = power on self test)
; BIOS searches for an OS to start
; BIOS loads and starts the OS
; OS runs
;
; How the BIOS finds and OS
; Legacy booting (legacy mode)
; - BIOS loads first sector of each bootable device into memory (at location 0x7C00)
; - BIOS checks for 0xAA55 signature at the end of the boot sector
; - If found, it starts executing code (jmp to the first instruction of the sector with the signature)
; EFI
; - BIOS looks into special EFI partitions
; - OS must be compiled as an EFI program
;
; Write asm code -> assembly it -> load in the first sector of a floppy disk

; The BIOS always put the OS at address 0x7C00, the first thing we need to do is give our assembler this information
; The ORG (directive): tells the assembler where we expect our code to be loaded.
; The assembler uses this information to calculate label addressses.
org 0x7C00

; Difference between Directive and Instruction
; Directive: 
; - Gives a clue to the assembler that will affect how the program gets compiled.
;   NOT translated to machine code!
; - Assembler specific - different assemblers may have different directives
; Instruction
; - Translated to a machine code instruction that the CPU will execute

; The BITS (directive): tells the assembler to emit 16/32/64 bit code
; Any x86 CPU must be backwards compatible with the original 8086 CPU
; If an OS written for the 8086 is run in a modern CPU it still needs to think that its running on a 8086
; because of this, the CPU always starts in 16 bit mode
; IMPORTANT: using "bits 32" won't make the processor run in 32 bit mode (just for the assembler)
bits 16

main: ; Where our code begins
  hlt

.halt: 
  jmp .halt

; We'll be putting our program on a 1.44 MB floppy disk where one sector has 512 bytes
; The BIOS expects that the last two bytes of the first sector are AA and 55 repectively
; For this we'll use another directive
; The DB: stands for "Declare constant Byte", write the given byte(s) to the assembled binary file
; The TIMES (directive): can be used to repeat the given instruction or piece of data a number of times
; Here we use it to pad our program so that it fills up to 510 bytes after which we declare the signature bytes
; The $: can be used to obtain the assembly position of the beggining of the current line
; The $$: give us the position of the current section
; $-$$: gives the size of out program so far
times 510 - ($ - $$) db 0
; We declare the signature
; The DW (signature): similar to DB but it declares a two byte constant (also known as a "word")
dw 0xAA55
