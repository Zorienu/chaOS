OUTPUT_FORMAT("binary")
/* src/bootloader/bootmain.c -> void loadOS () { ... } */
ENTRY(loadOS) 

/* Pointer to physical memory address 0x7C00,
 * which is where the BIOS loads our bootloader to and run at */
bootPhys = 0x7C00;

SECTIONS
{
  . = bootPhys;
  .boot : {
    *(.boot)
  }

  /* Put bootmain.c code at sector 1, just after the boot sector */
  .text : {
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

