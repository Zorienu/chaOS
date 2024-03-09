OUTPUT_FORMAT("binary")

/* Pointer to physical memory address 0x7C00,
 * which is where the BIOS loads our bootloader to (in RAM) and run at */
bootPhys = 0x7C00;

SECTIONS
{
  . = bootPhys;
  .boot : {
    *(.boot)
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

