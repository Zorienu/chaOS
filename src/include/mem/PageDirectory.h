#pragma once 

#include "virtualMem.h"
#include "PageDirectoryEntry.h"

// Page directory: handle 4GB each, 1024 page tables * 4MB
class PageDirectory2 {
  public:
    PageDirectoryEntry2 entries[TABLES_PER_DIRECTORY];
};
