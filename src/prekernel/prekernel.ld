OUTPUT_FORMAT("binary")

/* Pointer to physical memory address 0x7C00,
 * which is where the BIOS loads our bootloader to (in RAM) and run at */
prekernelPhysical = 0x50000;

SECTIONS
{
  . = prekernelPhysical;

  /* Put stage 2 code at sector 1, just after the boot sector */
  /* Skip some addresses to avoid affecting variables defined in boot.asm like SMAP_NUM_ENTRIES_ADDRESS and SMAP_ENTRIES_ADDRESS */
  .text : { 
    *(prekernel_entry);
    *(.text)
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }
}

