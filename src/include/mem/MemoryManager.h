#include <stdint.h>
#include "PhysicalAddress.h"
#include "VirtualAddress.h"
#include "PageTable.h"
#include "PageDirectory.h"

#define SMAP_NUM_ENTRIES_ADDRESS 0xA500;
#define SMAP_ENTRIES_ADDRESS     0xA504;

// extern uint32_t *kernelEnd;

typedef struct {
  uint64_t baseAddress; // Base address of the address range
  uint64_t length;      // Length of address range in bytes
  uint32_t type;        // Type of address range (enum )
  uint32_t acpi;
} SMAP_entry_t;

enum SMAP_entry_type {
  AVAILABLE           = 1, 
  RESERVED            = 2, // e.g. system ROM, memory-mapped devices
  ACPI_RECLAIM_MEMORY = 3, // Usable by OS after reading ACPI tables
  ACPI_NVS_MEMORY     = 4 // OS is required to save this memory between NVS sessions
};

#define MAX_BLOCKS_AMOUNT 1024 * 1024

class MemoryManager {
  public:
    void initialize(uint32_t kernelEnd);
    void print();


  private: 
    // Bitmap for telling which pages are free or not from the entire RAM
    uint32_t bitmap[MAX_BLOCKS_AMOUNT / 32];
    uint32_t availableBlocks;

    void initializeBlock(PhysicalAddress2 pa);
    void deinitializeBlock(PhysicalAddress2 pa);

    void map4MBPage(PageTable2 *pageTable, PhysicalAddress2 physicalFrame, VirtualAddress2 va);

    void loadPageDirectory2(PageDirectory2* pd);
    void enablePagination2();

    void *allocateBlock();


};
