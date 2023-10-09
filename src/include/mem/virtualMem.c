#include <stdint.h>
#include "virtualMem.h"
#include "mmu.h"

void setAttribute(PageTableEntry *entry, PAGE_TABLE_FLAGS attribute) {
  *entry |= attribute;
}

void setPhysicalFrame(PageTableEntry *entry, PhysicalAddress frame) {
  // Ox7FF = bits sets from bit 0 to bit 11 (flag bits)
  // Preserve flags and remove old frame
  // Set new frame
  *entry = (*entry & 0x7FF) | frame;
}
