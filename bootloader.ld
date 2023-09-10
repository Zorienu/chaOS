OUTPUT_FORMAT("binary")
/* src/bootloader/bootmain.c -> void loadOS () { ... } */
ENTRY(loadOS) 

/* Pointer to physical memory address 0x7C00,
 * which is where the BIOS loads our bootloader to and run at */
bootPhys = 0x7C00;
/* Pointer to physical memory address to 1MiB,
 * which is where our binary is to be loaded to and run at */
kernelPhys = 0x10000;

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

