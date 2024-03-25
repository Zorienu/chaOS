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
;  org 0x7C00

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

section .boot


; To print a new line we need to print both the line feed and the carriage return characters
; NASM macro
%define ENDL 0x0D, 0x0A

bootloader:

; Put here to avoid getting error "short jump is out of range"
start: 
  jmp main

; NOTE: use just int 16 bits code blocks
global halt
halt:
  mov bx, 0xDEAD ; Just to ensure we ended up here (use 'info registers' qemu cmd to confirm)
  cli
  hlt


%define DISK_PORT_BASE 0x1F0
%define SECTOR_SIZE 512
%define DISK_READ_CMD 0x20

; 
; Wait for the hard disk to be ready
;
wait_disk:
  push dx
  push ax

.loop:
  mov dx, DISK_PORT_BASE + 7 
  in al, dx
  and al, 0xC0
  cmp al, 0x40
  jne .loop

.done: 
  pop ax
  pop dx

  ret

; 
; Read sectors from disk using IO
; Parameters:
;   - bx: Sector number to read
;   - di: Memory address where to store the read data
;   - cx: Amount of sectors to read
;
disk_read:
  ; Save register we will modify
  push ax 
  push bx
  push cx
  push dx
  push di

  ; Wait for the disk
  call wait_disk

  ; Save amount of sectors to read... for later
  push cx

  ; Define number of sectors to read
  mov dx, DISK_PORT_BASE + 2
  mov ax, cx
  out dx, al

  ; Define sector to read
  mov dx, DISK_PORT_BASE + 3
  mov al, bl ; LSB of bx
  out dx, al

  mov dx, DISK_PORT_BASE + 4
  mov al, bh ; MSB of bx
  out dx, al

  mov dx, DISK_PORT_BASE + 5
  mov al, 0 ; sector 0
  out dx, al

  mov dx, DISK_PORT_BASE + 6
  mov al, 0xE0 ; 0xE0 -> 0b1110_0000
               ; bit 4: drive number (0 in our case)
               ; bit 5: always set
               ; bit 6: set for LBA
               ; bit 7: always set
  or al, 0 ; sector 0
  out dx, al

  ; Send read command
  mov dx, DISK_PORT_BASE + 7
  mov al, DISK_READ_CMD
  out dx, al

  ; Wait for the disk
  call wait_disk

  ; The division puts the remainder in the edx register, so
  ; we have to tell the disk port after the division
  mov dx, DISK_PORT_BASE ; Port to read

  ; cld
  ; ; rep insd ; Reading DWORDS (32 bits) repeatedly until cx (number of times we have to read) reaches 0
  ; rep insw ; Reading WORDS (16 bits) repeatedly until cx (number of times we have to read) reaches 0

  ; Restore previously saved "amount of sectors to read" into bx
  pop bx

; Loop for every sector we're gonna read
; NOTE: this worked with cld; rep insw and cx holding the total amount of bytes to read / 2 (4 in case of insd)
; in bochs but not in qemu. So, we had to read sectors one by one adding a wait for disk
.loop:
  ; How many times are we gonna read from the port?
  ; cx register will contain the counter...
  ; insd reads 32 bits at a time (4 bytes)
  ; insw reads 16 bits at a time (2 bytes)
  ; so, we have to divide the amount of bytes to read by 4 or 2 depending of the used command
  mov cx, SECTOR_SIZE / 2

  cld
  ; rep insd ; Reading DWORDS (32 bits) repeatedly until cx (number of times we have to read) reaches 0
  rep insw ; Reading WORDS (16 bits) repeatedly until cx (number of times we have to read) reaches 0

  call wait_disk

  ; Have we read all the sectors?
  cmp bl, 0
  je .done

  ; We read 1 sector, then, decrement the counter
  dec bx
  jmp .loop

.done:
  ; Restore registers we modified
  pop di 
  pop dx
  pop cx
  pop bx
  pop ax

  ret
  

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


bits 16
; One sector after the bootsector code loaded in RAM by the BIOS
%define STAGE_2_ADDRESS 0x7E00
main: ; Where our code begins
  ; We don't know if the DS (Data Segment) or the ES (Extra Segment) registers are properly initialized
  ; Since we can't write a constant directly to registers, we have to use an intermediary register (ax)
  mov ax, 0
  mov ds, ax
  mov es, ax

  ; Setup the SS (Stack Segment) and the stack pointer to the begin of our program
  mov ss, ax
  mov sp, bootloader ; Stack grows downwards from where we are loaded in memory

  mov si, msg_hello
  call puts

  mov di, STAGE_2_ADDRESS ; di will contain the destination address of the read sectors
  mov cx, 7 ;* SECTOR_SIZE ; cx will contain the number of bytes to read (60 sectors)
  mov bx, 1 ; from which sector we want to read?
  call disk_read
  
  call STAGE_2_ADDRESS

  call halt

; Declare string variable
msg_hello: db "Hello world!", ENDL, 0x0
msg_error_reading_from_disk: db "Read from disk failed!", ENDL, 0x0

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
