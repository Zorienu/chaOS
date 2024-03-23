OUTPUT_FORMAT("binary")

/* Pointer to physical memory address 0x7C00,
 * which is where the BIOS loads our bootloader to (in RAM) and run at */
bootPhys = 0x7C00;
stage2Physical = 0x7E00;

SECTIONS
{
  . = bootPhys;
  .boot : {
    *(.boot)
  }

  . = stage2Physical;
  .stage2 : {
    *(.stage2)
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

