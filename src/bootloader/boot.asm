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
; NOTE: The BIOS automatically sets up the CS (Code Segment) register to segment 0 (with 0x7C00 offset)
org 0x7C00

; The BITS (directive): tells the assembler to emit 16/32/64 bit code
; Any x86 CPU must be backwards compatible with the original 8086 CPU
; If an OS written for the 8086 is run in a modern CPU it still needs to think that its running on a 8086
; because of this, the CPU always starts in 16 bit mode
; IMPORTANT: using "bits 32" won't make the processor run in 32 bit mode (just for the assembler)
bits 16


; To print a new line we need to print boot the line feed and the carriage return characters
; NASM macro
%define ENDL 0x0D, 0x0A

; -----------------------------------------------------------------------------------------------
; 
; Define FAT12 header
;
; jmp over the disk format information, since the first sector of the disk is loaded into RAM
; at location 0x0000:0x7C00 and executed, without this jmp, the CPU will attempt to execute data that isn't code
jmp short start 
nop 

; BPB (BIOS Parameter Block)
bpb_oem_identifier:                 db "MSWIN4.1" ; 8 bytes
bpb_number_of_bytes_per_sector:     dw 512 ; This is represented as 00 02 in little endian which is 0x200 = 512
bpb_number_of_sectors_per_cluster:  db 1
bpb_number_of_reserved_sectors:     dw 1
bpb_fat_count:                      db 2
bpb_dir_entries_count:              dw 0xE0
bpb_total_sectors:                  dw 2880 ; This is represented as 40 0B in little endian which is 0xB40 = 2880
bpb_media_descriptor_type:          db 0xF0 ; 0xF0 = 3.5" floppy disk
bpb_number_of_sectors_per_fat:      dw 9 ; This is represented as 09 00 in little endian which is 0x09 = 9
bpb_number_of_sectors_per_track:    dw 18 ; This is represented 12 00 in little endian which is 0x12 = 18
bpb_number_of_heads:                dw 2 ; This is represented 02 00 in little endian which is 0x02 = 2
bpb_hidden_sector_count:            dd 0 
bpb_large_sector_count:             dd 0

; EBR (Extended Boot Record)
ebr_drive_number:                   db 0 ; 0x00 floppy, 0x80 hard disks. This number is useless because the media 
                                 ; is likely to be moved to another machine and inserted in a drive with a different drive number.
                                    db 0 ; Reserved
ebr_signature:                      db 0x29 ; Must be 0x28 or 0x29
ebr_volume_serial_number:           db 0x78, 0x56, 0x34, 0x12 ; Hex to get 12345678 serial, does not matter
ebr_volume_label_string:            db '   CHAOS   ' ; 11 bytes string padded with spaces
ebr_system_id:                      db ' FAT 12 ' ; 8 bytes string padded with spaces
;
; End of defining FAT12 header
;
; -----------------------------------------------------------------------------------------------



; Difference between Directive and Instruction
; Directive: 
; - Gives a clue to the assembler that will affect how the program gets compiled.
;   NOT translated to machine code!
; - Assembler specific - different assemblers may have different directives
; Instruction
; - Translated to a machine code instruction that the CPU will execute

start: 
  jmp main

;
; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts: 
  ; save registers we will modify 
  push si
  push ax

.loop:
  ; LODSB, LODSW, LODSD: these instructions load a byte/word/double-word from DS:SI into AL/AX/EAX, 
  ; then increment SI by the number of loaded bytes
  ; In this case we use LODSB because each character occupies a byte
  lodsb

  ; OR destination, source: performs bitwise OR between source and destination, stores result in destination
  ; NOTE: OR the value to itself won't modify the value but will modify some flags register,
  ; such as the zero flag if the result is 0
  or al, al
  ; If the next character is the NULL character, then we're done
  jz .done

  ; The BIOS provides basic functions which allow us to do basic stuff such as writing text to the screen
  ; by using interrupts
  mov ah, 0x0E ; Write character in TTY mode 
  mov bh, 0 ; Set the page number to 0
  int 0x10 ; Call interrupt for video services

  ; Continue with the next character (byte)
  jmp .loop

.done: 
  pop ax
  pop si
  ret


main: ; Where our code begins
  ; We don't know if the DS (Data Segment) or the ES (Extra Segment) registers are properly initialized
  ; Since we can't write a constant directly to registers, we have to use an intermediary register (ax)
  mov ax, 0
  mov ds, ax
  mov es, ax

  ; Setup the SS (Stack Segment) and the stack pointer to the begin of our program
  mov ss, ax
  mov sp, 0x7C00 ; Stack grows downwards from where we are loaded in memory
  
  mov si, msg_hello
  call puts


  hlt

.halt: 
  jmp .halt

; Declare string variable
msg_hello: db "Hello world!", ENDL, 0x0


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
; NOTE: the bytes are in reverse order because x86 is little endian
dw 0xAA55
