#pragma once 

#include "PageTableEntry.h"

// Page table: handle 4MB each, 1024 entries * 4096
class PageTable2 {
  public:
    PageTableEntry2 entries[PAGES_PER_TABLE];
};
