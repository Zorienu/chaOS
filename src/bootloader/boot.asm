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


; Put here to avoid getting error "short jump is out of range"
start: 
  jmp main

; -----------------------------------------------------------------------------------------------
;
; Error handlers
;
;
; Displays an error if failed reading sector from disk
;
floppy_error:
  mov si, msg_error_reading_from_disk
  call puts
  jmp wait_key_and_reboot

wait_key_and_reboot:
  mov ah, 0
  int 0x16 ; Wait for key press
  jmp 0xFFFF ; Jump to beggining of the BIOS, should reboot

halt:
  cli
  hlt
;
; End of defining error handlers
;
; -----------------------------------------------------------------------------------------------

; -----------------------------------------------------------------------------------------------
;
; Disk routines
; 

; 
; LBA address to CHS conversion
; Parameters: 
;   - ax: LBA address
; Returns:
;   - cx [bits 0 - 5]: sector number
;   - cx [bits 6 - 15]: cylinder number
;   - dh: head
;
lba_to_chs:
  push ax
  push dx

  xor dx, dx ; dx = 0
  div word [bpb_number_of_sectors_per_track] ; ax = LBA / sectorsPerTrack
                                             ; dx = LBA % sectorsPerTrack

  inc dx ; dx = dx + 1 = LBA % sectorsPerTrack + 1 = sector number
  mov cx, dx ; cx = sector number

  xor dx, dx ; dx = 0
  div word [bpb_number_of_heads] ; ax = LBA / sectorsPerTrack / numberOfHeads = cylinder number
                                 ; dx = LBA / sectorsPerTrack % numberOfHeads = head number (in dl actually)

  mov dh, dl ; dh = head number

  ; ax ---ah--- | ---al---
  ;    xxxxxx98 | 76543210
  ; because is little endian we have to end up with the cylinder number in cx like this
  ; cx ---ch--- | ---cl---
  ;    76543210 | 98--dx-- Where dx contains the sector number
  shl ah, 6 ; ah << 6: xxxxxx98 => 98000000
  or cl, ah ; Put upper 2 bits of the cylinder in cx: 98000000 OR sector number
  mov ch, al ; Put lower 8 bits of the cylinder: ch = 76543210

  pop ax ; Pop dx into ax
  mov dl, al ; We put the head number in dh, so we restore dl only, 
             ; since we cannot push 8 bit values to the stack, we pushed the hole register and get dl later
  pop ax

  ret

; 
; Read sectors from disk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read
;   - dl: drive number
;   - es:bx: Memory address where to store the read data
; Returns:
;   - cf: Set on error, clear if no error
;   - ah: Return code
;   - al: Actual sectors read count
;
disk_read:
  ; Save register we will modify
  push ax 
  push bx
  push cx
  push dx
  push di

  push cx ; Temporarily save cl (number of sectors to read), since the lba_to_chs routine will write to it

  call lba_to_chs ; Compute CHS values

  pop ax ; al = number of sectors to read (pushed cl previously)
  mov ah, 0x02 ; Read sectors from drive

  mov di, 3 ; Initialize retries for reading from disk to 3

.retry:
  pusha ; Save all registers, since we don't know what BIOS modifies
  stc ; Set carry flag, some BIOS'es don't set it

  int 0x13 ; Call the interrupt, Carry flag (cf) is clear, then success, else error
  jnc .done ; Jump near if not carry (cf = 0)

  ; Read disk failed, reset disk controller
  popa 
  call disk_reset

  dec di ; Consume 1 retry
  test di, di ; Still have retries?
  jnz .retry ; If attempts remaining, then retry

.fail
  ; All attempts exhausted
  jmp floppy_error

.done:
  popa

  ; Restore registers we modified
  pop di 
  pop dx
  pop cx
  pop bx
  pop ax

  ret


;
; Resets disk controller
; Parameters:
;  - dl: drive number
;
disk_reset:
  pusha 

  mov ah, 0
  stc
  int 0x13
  jc floppy_error

  popa
  ret
;
; End of defining disk routines
;
; -----------------------------------------------------------------------------------------------

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

  ; Read something from floppy disk
  mov [ebr_drive_number], dl ; BIOS should set dl to drive number
  mov ax, 1 ; LBA=1, second sector from disk
  mov cl, 1 ; Number of sectors to read (just 1)
  mov bx, 0x7E00 ; Put the read sector after the bootloader code
  call disk_read
  
  mov si, msg_hello
  call puts

  call halt

; Declare string variable
msg_hello: db "Hello world!", ENDL, 0x0
msg_error_reading_from_disk: db "Read from disk failed!", ENDL, 0x0
msg_test: db "test", ENDL, 0x0


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
