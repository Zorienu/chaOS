#include <stdint.h>
#include "virtualMem.h"
#include "mmu.h"

void setAttribute(PageTableEntry *entry, PAGE_TABLE_FLAGS attribute) {
  *entry |= attribute;
}
