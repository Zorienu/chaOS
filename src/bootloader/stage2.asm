;
; Use another asm file since all the code we need does not fit in the bootloader (which is just 512 bytes)
; This will:
; - Get the memory size and memory map
; - Enter protected mode
; - call loadOS at src/bootloader/bootmain.c
;

extern halt
bits 16

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


;
; Already in protected mode
;
bits 32
extern bootloader
PModeMain:
  ; Setup segments register to 16 (0x10) which is the offset within the GDT to use the kernel mode data segment
  mov ax, GDT_KERNEL_MODE_DATA_SEGMENT_ENTRY 
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  mov esp, bootloader ; Set the stack at 0x7C00

  call OSEntry
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

global stage2
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

; OS bootloader code (bootmain.c)
bits 32

extern loadOS

section .text

OSEntry:
  call loadOS
  cli 
  hlt

times 512 - ($ - $$) db 0
