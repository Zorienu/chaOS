OUTPUT_FORMAT("binary")
/* src/kernel/main.c -> void OSStart () { ... } */
ENTRY(OSStart) 

/* Pointer to physical memory address to 1MiB,
 * which is where our binary is to be loaded to and run at */
phys = 0x7C00;

SECTIONS
{
  . = phys;

  .boot : {
    *(.boot)
    *(.text)
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  . = 0x7C00 + 0x100000;

  .bss : {
    *(.bss)
  }
}

