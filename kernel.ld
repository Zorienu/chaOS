OUTPUT_FORMAT("elf32-i386")
/* src/kernel/entry.asm -> entry */
/* Kernel will be loaded at 3GB virtual address */
KERNEL_BASE = 0xC0000000;

/* 
 * This replaces ENTRY(...)
 * Point the entry point to the physical address where entry.asm starts
 * This is needed becauses at this point we haven't enable paging yet
 */
start = entry - KERNEL_BASE;

/* 
 * Where in physical memory the kernel code will be placed 
 */
kernelPhys = 0x100000;

/* 
 * Where in virtual memory the kernel code will be placed
 * placed 0x100000 above to avoid I/O space
 */
kernelVirtual = KERNEL_BASE + kernelPhys;

SECTIONS
{
  . = kernelVirtual;

  .text : AT(kernelPhys) {
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

  /* This symbol will let us know where in physical memory the kernel ends */
  PROVIDE(kernelEnd = .);
}

