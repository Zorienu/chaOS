OUTPUT_FORMAT("binary")
/* src/kernel/main.c -> void OSStart () { ... } */
ENTRY(OSStart) 

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

  . = kernelPhys;
  /* Put kernel code at sector 1, just after the boot sector
   * But load it in RAM at address 0x10000 
   * (second sector in disk) */
  .text kernelPhys : AT(ADDR(.boot) + 512) {
    *(.text)
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  . = kernelPhys + 0x200000;
  .bss : {
    *(.bss)
  }
}

