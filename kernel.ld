OUTPUT_FORMAT("elf32-i386")
/* src/kernel/main.c -> void OSStart () { ... } */
ENTRY(OSStart) 

kernelPhys = 0x100000;

SECTIONS
{
  . = kernelPhys;
  /* Put kernel code at sector 1, just after the boot sector
   * But load it in RAM at address 0x10000 
   * (second sector in disk) */
  .text : {
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

