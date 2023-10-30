#include <stdint.h>

#define MB 0x100000
#define PAGE_SIZE 4096
#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIRECTORY 1024

#define REMOVE_ATTRIBUTE (entry, attribute) (*entry &= ~attribute)
#define TEST_ATTRIBUTE (entry, attribute) (*entry & attribute)

typedef uint32_t PageDirectoryEntry;
typedef uint32_t PageTableEntry;
typedef uint32_t VirtualAddress;
typedef uint32_t PhysicalAddress;

typedef enum {
    PTE_PRESENT       = 0x01,
    PTE_READ_WRITE    = 0x02,
    PTE_USER          = 0x04,
    PTE_WRITE_THROUGH = 0x08,
    PTE_CACHE_DISABLE = 0x10,
    PTE_ACCESSED      = 0x20,
    PTE_DIRTY         = 0x40,
    PTE_PAT           = 0x80,
    PTE_GLOBAL        = 0x100,
    PTE_FRAME         = 0x7FFFF000,   // bits 12+
} PAGE_TABLE_FLAGS;

typedef enum {
    PDE_PRESENT       = 0x01,
    PDE_READ_WRITE    = 0x02,
    PDE_USER          = 0x04,
    PDE_WRITE_THROUGH = 0x08,
    PDE_CACHE_DISABLE = 0x10,
    PDE_ACCESSED      = 0x20,
    PDE_DIRTY         = 0x40,          // 4MB entry only
    PDE_PAGE_SIZE     = 0x80,          // 0 = 4KB page, 1 = 4MB page
    PDE_GLOBAL        = 0x100,         // 4MB entry only
    PDE_PAT           = 0x2000,        // 4MB entry only
    PDE_FRAME         = 0x7FFFF000,    // bits 12+
} PAGE_DIRECTORY_FLAGS;


// Page table: handle 4MB each, 1024 entries * 4096
typedef struct {
    PageTableEntry entries[PAGES_PER_TABLE];
} PageTable;

// Page directory: handle 4GB each, 1024 page tables * 4MB
typedef struct {
    PageDirectoryEntry entries[TABLES_PER_DIRECTORY];
} PageDirectory;


/*
 * Set the given paging attribute to the given page table entry
 */
void setAttribute(PageTableEntry *entry, PAGE_TABLE_FLAGS attribute);

/*
 * Set the given frame as the physical frame for the given page table entry
 */
void setPhysicalFrame(PageTableEntry *entry, PhysicalAddress frame);

/*
 * Load the given page directory to the CR3 register
 */
void loadPageDirectory(PageDirectory *pageDirectory);

/*
 * Enable pagination
 */
void enablePagination();

/*
 * Get the physical memory address of the page table pointed by the given page directory entry
 */
PageTable *getPagePhysicalAddress(PageDirectoryEntry *entry);

/*
 * Get the current active page directory
 */
PageDirectory *getPageDirectory();

/*
 * Map the given virtual address to the given physical address
 * in the current page directory
 */
VirtualAddress mapPage(VirtualAddress virtualAddress, PhysicalAddress physicalAddress);

/*
 * Map the reserved quickmap page table to the given physical address
 */
VirtualAddress quickmapPage(PhysicalAddress physicalAddress);
