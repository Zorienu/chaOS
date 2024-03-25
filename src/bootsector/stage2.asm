;
; Use another asm file since all the code we need does not fit in the bootloader (which is just 512 bytes)
; This will:
; - Get the memory size and memory map
; - Enter protected mode
; - call loadOS at src/bootloader/bootmain.c
;

bits 16

;
; We do this to avoid overlapping with the memory map addresses
; when loading the code into RAM (memmap_entries 0xA500)
;
section .stage2
jmp stage2

extern halt

; -----------------------------------------------------------------------------------------------
; 
; A20 line configuration
;
; 
; Test whether the A20 address line was already enabled by the BIOS
; When accessing RAM with A20 disabled, any address above 1MiB wraps around to zero
; Returns:
;   - ax: A20 enabled? 1 -> yes, 0 -> no
;
testA20Line:
  push ds
  push es
  push di
  push si

  cli ; Disable interrupts

  xor ax, ax ; ax = 0
  mov es, ax ; set es segment to be 0x0000 (segment 0 which is physical 0)

  not ax ; ax = 0xFFFF
  mov ds, ax ; set ds segment to be 0xFFFF (segment 65535 which is physical address 1048560)

  mov di, 0x0500 ; Offset for segment 0, resulting in physical address: segment 0 * 16 + 1280 = 1280 = 0x500
  mov si, 0x0510 ; Offset for segment 65535, resulting in physical address: segment 65535 * 16 + 1296 = 1049856 = 0x100500
                 ; We can get this offset from doing: (0x500 + 0x10000) (we want 1MiB above) - (the segment * 16) =
                 ;                                    (0x100500) - (0xFFFF * 16) =
                 ;                                    (0x100500) - (0xFFFF0) = 0x510

  mov al, [es:di] ; Get the current value stored in 0x0000:0x0500
  push ax ; Store it into the stack temporarily

  mov al, [ds:si] ; Get the current value stored in 0xFFFF:0x0510
  push ax ; Store it into the stack temporarily

  mov byte [es:di], 0x00 ; Set the value of 0x0000:0x0500 to 0x00
  mov byte [ds:si], 0xFF ; Set the value of 0xFFFF:0x0510 to 0xFF (something totally different)

  cmp byte [es:di], 0xFF ; Compare the value stored in 0x000:0x0500 (which is 0x00) to 0xFF
                         ; If equal, it means the A20 line is not enabled (is like we are seeing 0x0000:0x0500),
                         ; otherwise is activated

  pop ax
  mov byte [ds:si], al ; Restore the previously saved value of 0xFFFF:0x0500
  pop ax
  mov byte [es:di], al ; Restore the previously saved value of 0x0000:0x0500

  mov ax, 0
  je .testA20LineExit

  mov ax, 1

.testA20LineExit:
  pop si
  pop di
  pop es
  pop ds

  ret

; 
; Enable A20 line if not enabled yet
;
enableA20Line:
  push ax

  call testA20Line
  test ax, ax ; ax & ax === 1 ? ZF = 0 : ZF = 1
  jnz .endEnableA20Line

  in al, 0x92
  or al, 2
  out 0x92, al

.endEnableA20Line:
  pop ax
  ret
;
; End of A20 line configuration
;
; -----------------------------------------------------------------------------------------------

; -----------------------------------------------------------------------------------------------
; 
; Protected mode configuration
;
%define GDT_KERNEL_MODE_CODE_SEGMENT_ENTRY 8
%define GDT_KERNEL_MODE_DATA_SEGMENT_ENTRY 16
;
; Enter protected mode
;
bits 16
enterProtectedMode: 
  cli ; Disable interrupts
  call enableA20Line
  lgdt [GDTDescriptor] ; Load GDT register with start address of GDT

  mov eax, cr0
  or al, 1 ; Set PE (Protection Enable) bit in CR0 (Control Register 0)
  mov cr0, eax 

  jmp GDT_KERNEL_MODE_CODE_SEGMENT_ENTRY:PModeMain

  call halt


%define DISK_PORT_BASE 0x1F0
%define SECTOR_SIZE 512
%define DISK_READ_CMD 0x20

; 
; Wait for the hard disk to be ready
;
bits 32
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
bits 32
disk_read:
  ; Save register we will modify
  push ax 
  push bx
  push cx
  push dx
  push di

  ; Wait for the disk
  call wait_disk

  ; Save sector number to read
  ; push bx

  ; Convert number of bytes to number of sectors to read
  ; Number of sectors to read will be in ax and remainder in dx
  ; mov ax, cx
  ; mov bx, SECTOR_SIZE
  ; cwd ; prepare the DX:AX register pair for division -> dx will be set to 0
      ;  setting dx to 0 manually will work too
  ; div bx

  ; Restore sector number to read
  ; pop bx

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
; Already in protected mode
;
bits 32
%define SECTORS_PER_BLOCK 8
%define BLOCK_SIZE 4096
%define SUPERBLOCK_DISK_BLOCK 1
%define PREKERNEL_MEMORY_ADDRESS 0x50000
%define SUPERBLOCK_MEMORY_ADDRESS 0x8E00
%define INODES_MEMORY_ADDRESS SUPERBLOCK_MEMORY_ADDRESS + BLOCK_SIZE
%define FIRST_INODE_DISKBLOCK_ADDRESS SUPERBLOCK_MEMORY_ADDRESS + 24 ; Offset of "firstInodeBlock" within superBlock struct
%define PREKERNEL_INODE 3
%define INODE_STRUCT_SIZE 128
%define INODE_FIRST_DIRECT_DATA_BLOCK_OFFSET 8 ; Offset of "directDataBlocks" within inode struct
%define INODE_SIZE_IN_SECTORS_OFFSET 113 ; Offset of "sizeInSectors" within inode struct
PModeMain:
  ; Setup segments register to 16 (0x10) which is the offset within the GDT to use the kernel mode data segment
  mov ax, GDT_KERNEL_MODE_DATA_SEGMENT_ENTRY 
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ;
  ; Bootloader starts at address 0x7C00, we will use this for setting the stack
  ; since we don't need the content before this physical address 
  ; remember: the stack grows downwards
  ;
  mov esp, 0x7C00

  ; Load superblock to memory
  mov di, SUPERBLOCK_MEMORY_ADDRESS
  mov cx, SECTORS_PER_BLOCK ; Convert disk block to sector number (each block is 8 sectors)
  mov bx, SUPERBLOCK_DISK_BLOCK * SECTORS_PER_BLOCK ; Convert disk block to sector number (each block is 8 sectors)
  call disk_read

  ; Get the block where the first inode is located from the superblock
  ; and load it into memory
  mov di, INODES_MEMORY_ADDRESS
  mov cx, SECTORS_PER_BLOCK; BLOCK_SIZE in sectors
  mov bx, [FIRST_INODE_DISKBLOCK_ADDRESS]
  imul bx, SECTORS_PER_BLOCK ; Convert first inode block to sector number
  call disk_read

  mov edi, PREKERNEL_MEMORY_ADDRESS
  ; Get the first disk block of the prekernel
  mov bx, [INODES_MEMORY_ADDRESS + PREKERNEL_INODE * INODE_STRUCT_SIZE + INODE_FIRST_DIRECT_DATA_BLOCK_OFFSET]
  imul bx, SECTORS_PER_BLOCK
  ; Get the size in sectors of the prekernel
  mov cx, [INODES_MEMORY_ADDRESS + PREKERNEL_INODE * INODE_STRUCT_SIZE + INODE_SIZE_IN_SECTORS_OFFSET]
  call disk_read

  call PREKERNEL_MEMORY_ADDRESS
;
; End of protected mode configuration
;
; -----------------------------------------------------------------------------------------------

; -----------------------------------------------------------------------------------------------
;
; RAM utils
;
;
; Get memory size
; returns:
;  - cf: clear if successful
;  - eax: extended memory between 1MB and 16MB in KB
;  - ebx: extended memory above 16MB in 64KB blocks (to know the amount of KB, multiply by 64)
;  - ecx: configured memory 1MB to 16MB in KB
;  - edx: configured memory above 16MB in 64KB blocks (to know the amount of KB, multiply by 64)
bits 16
memorySize equ 0xA4FC ; Store number of memory map entries here

getMemorySize:
  push ecx
  push edx

  xor ecx, ecx ; Clear all registers
  xor edx, edx

  mov ax, 0xE801 ; Function 0xE801 - Get memory size for > 64MB configurations
  int 0x15

  jc .error

  cmp ah, 0x86 ; unsupported function
  je .error

  cmp ah, 0x80 ; invalid command
  je .error

  jcxz .endGetMemorySize ; BIOS may have stored the result in ax, bx or cx, dx. Test if cx is 0
                         ; if not, cx should contain memory size, store it
  mov ax, cx
  mov bx, dx

  jmp .endGetMemorySize

.error:
  mov ax, -1
  mov bx, 0

.endGetMemorySize:
  mov [memorySize], eax

  pop edx
  pop ecx
  ret

; 
; Address range descriptor; The buffer used by this interrupt as an array of descriptors that follow the following format:
;
struc MemoryMapEntry
  .baseAddress resq 1 ; Base address of the address range
  .length      resq 1 ; Length of address range in bytes
  .type        resd 1 ; Type of address range:
                      ; - 1: Available Memory
                      ; - 2: Reserved, do not use. (e.g. system ROM, memory-mapped device)
                      ; - 3: ACPI Reclaim Memory (usable by OS after reading ACPI tables)
                      ; - 4: ACPI NVS Memory (OS is required to save this memory between NVS sessions)
                      ; All other values should be treated as undefined.
  .acpi_null   resd 1 ; Reserved
endstruc

;
; Get memory map: this will let us know what memory is available for use and what is reserved
; params:
;  - eax: 0x0000E820
;  - ebx: continuation value or 0 to start the beginning of map
;  - ecx: size of bugger for result (Must be >= 20 bytes)
;  - edx: 0x534D4150 ('SMAP')
;  - es:di: buffer for result
; returns: 
;  - cf: clear if successful
;  - eax: 0x534D4150h ('SMAP')
;  - ebx: offset of the next entry to copy from or 0 if done
;  - ecx: actual length returned in bytes
;  - es:di: buffer filled
;  - ah: if error, contains error code
memmap_entries equ 0xA500 ; Store number of memory map entries here

getMemoryMap:
	pushad

	mov di, 0xA504      ; Memory map entries start here

	xor	ebx, ebx        ; ebx = 0 to start, will contain continuation values
	xor	bp, bp			    ; bp will contain the number of entries
	mov	edx, 'PAMS'	  	; 'SMAP' in little endian
	mov	eax, 0xE820
	mov	ecx, 24			    ; Memory map entry struct is up to 24 bytes
	int	0x15			      ; Get first entry

	jc	.error	        ; If carry is set, function not supported or errored

	cmp	eax, 'PAMS'		  ; BIOS returns SMAP in eax on successful call
	jne	.error

	test	ebx, ebx		  ; Does ebx = 0? if so only 1 entry or no entries :(
	je	.error
	jmp	.start          ; ebx != 0, have a valid entry

.next_entry:
	mov	edx, 'PAMS'		  ; Some BIOS's trash this register, reset this to 'SMAP'
	mov	ecx, 24			    ; Reset ecx, Entry is 24 bytes
	mov	eax, 0xE820     ; Reset eax
	int	0x15			      ; Get next entry

.start:
	jcxz	.skip_entry		; Memory map entry is 0 bytes in length, skip

.notext:
	mov	ecx, [es:di + MemoryMapEntry.length]	    ; Get length (low dword)
	test	ecx, ecx		                            ; If length is 0, skip
	jne	short .good_entry
	mov	ecx, [es:di + MemoryMapEntry.length + 4]  ; Get length (upper dword)
	jecxz	.skip_entry		                          ; If length is 0, skip
	  
.good_entry:
	inc	bp			                                  ; Increment entry count
	add	di, 24			                              ; Point di to next entry in buffer
	  
.skip_entry:
	cmp	ebx, 0			                              ; If ebx != 0, still have entries to read 
	jne	.next_entry		                            ; Get next entry
	jmp	.done
	  
.error:
	stc
	
.done:
  mov [memmap_entries], bp                      ; Store the number of entries in 0xA500
	popad
	ret

;
; End of RAM utils
;
; -----------------------------------------------------------------------------------------------

stage2: 
  call getMemorySize
  call getMemoryMap
  call enterProtectedMode


;
; Declare GDT segment descriptors
;
; Access type byte:
; - P (7): Present bit, 1 for any valid segment
; - DPL (6-5): Descriptor Privilege Level field, CPU privilege level of the segment
;              0 (0b00): highest privilege (kernel)
;              3 (0b11): lowest privilege (user apps) 
; - S (4): descriptor type bit
;          0: system segment (e.g. a Task State Segment)
;          1: defines a code or data segment
; - E (3): executable bit:
;          0: data segment (not executable)
;          1: code segment (executable)
; - DC (2): Direction bit / Conforming bit
;           - For data selectors: Direction bit:
;                                 0: the segment grows up
;                                 1: the segment grows down
;           - For code selectors: Conforming bit:
;                                 0: code in this segment can be executed from the ring set in DPL
;                                 1: code in this segment can be executed from an equal or lower privilege level, e.g.
;                                    code in ring 3 (lower privilege) can far-jmp to conforming code in a ring 2 segment
;                                    The DPL represents the highest privelege level that is allowed to execute the segment.
;                                    For example, code in ring 0 cannot far-jmp to a conforming code segment where DPL is 2,
;                                    while code in ring 2 or 3 can.
;                                    Note that the privelege level remains the same, ie. a far-jmp from ring 3 to a segment
;                                    with a DPL of 2 remains in ring 3 after the jmp
; - RW (1): Readable bit/Writeable bit
;           - For code segments: Readable bit (Write access is never allowed for code segments)
;                                0: read access for this segment is not allowed
;                                1: read access is allowed
;           - For data segments: Writeable bit (Read access is always allowed for data segments)
;                                0: write access for this segment is not allowed
;                                1: write access is allowed
; - A (0): Accessed bit: best left clear (0), the CPU will set it when the segment is accessed
GDTStart:
.GDTNullSegment: dq 0 ; 8 bytes in 0
.GDTKernelModeCodeSegment: dw 0xFFFF ; Limit
                           dw 0x0000 ; Base
                           db 0x00   ; Base
                           db 0b10011010 ; Access type (0x9A)
                           db 0b11001111 ; Flags (0xC) (A nibble) - Limit (0xFF) (A nibble)
                           db 0x00 ; Base

.GDTKernelModeDataSegment: dw 0xFFFF ; Limit                                                
                           dw 0x0000 ; Base                                                 
                           db 0x00   ; Base                                                 
                           db 0b10010010 ; Access type (0x92)                               
                           db 0b11001111 ; Flags (0xC) (A nibble) - Limit (0xFF) (A nibble) 
                           db 0x00 ; Base                                                   

.GDTUserModeCodeSegment:   dw 0xFFFF ; Limit                                                 
                           dw 0x0000 ; Base                                                  
                           db 0x00   ; Base                                                  
                           db 0b11111010 ; Access type (0xFA)
                           db 0b11001111 ; Flags (0xC) (A nibble) - Limit (0xFF) (A nibble)  
                           db 0x00 ; Base                                                    

.GDTUserModeDataSegment:   dw 0xFFFF ; Limit                                                 
                           dw 0x0000 ; Base                                                  
                           db 0x00   ; Base                                                  
                           db 0b11110010 ; Access type (0xF2)
                           db 0b11001111 ; Flags (0xC) (A nibble) - Limit (0xFF) (A nibble)  
                           db 0x00 ; Base                                                    
GDTEnd:

GDTDescriptor:
.GDTSize:    dw GDTEnd - GDTStart
.GDTAddress: dd GDTStart

times 1024 - ($ - $$) db 0
