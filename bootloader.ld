OUTPUT_FORMAT("binary")
/* src/bootloader/bootmain.c -> void loadOS () { ... } */
ENTRY(loadOS) 

/* Pointer to physical memory address 0x7C00,
 * which is where the BIOS loads our bootloader to (in RAM) and run at */
bootPhys = 0x7C00;

SECTIONS
{
  . = bootPhys;
  .boot : {
    *(.boot)
  }

  /* Put bootmain.c code at sector 1, just after the boot sector */
  /* Skip some addresses to avoid affecting variables defined in boot.asm like SMAP_NUM_ENTRIES_ADDRESS and SMAP_ENTRIES_ADDRESS */
  .text 0xB000 : { 
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

