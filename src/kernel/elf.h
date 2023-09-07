#include <stdint.h>
#include <stdbool.h>
 
typedef uint16_t ELF32_Half;	// Unsigned half int
typedef uint32_t ELF32_Offset;	  // Unsigned offset
typedef uint32_t ELF32_Address;	// Unsigned address
typedef uint32_t ELF32_Word;	// Unsigned int
typedef int32_t  ELF32_SWord;	// Signed int

/*
 * 16 first bytes within the ELF file which 
 * provide important information about the ELF file such as the intended architecture, 
 * byte order, and ABI information
 */
#define ELF_NIDENT 16

/*
 * First bytes within the ELF file header
 * These are the important ones for x86
 */
enum ELF_Ident {
	EI_MAGIC_0		  = 0, // 0x7F
	EI_MAGIC_1		  = 1, // 'E'
	EI_MAGIC_2		  = 2, // 'L'
	EI_MAGIC_3		  = 3, // 'F'
	EI_CLASS	      = 4, // Architecture (32/64)
	EI_DATA		      = 5, // Byte Order (litte or big endian)
	EI_VERSION	    = 6, // ELF Version
	EI_OS_ABI	      = 7, // OS Specific
	EI_ABI_VERSION	= 8, // OS Specific
	EI_PADDING		  = 9  // Padding
};
 
/*
 * ELF magic signature
 */
#define ELF_MAGIC_0	0x7F // ELF_header->e_ident[EI_MAGIC_0]
#define ELF_MAGIC_1	'E'  // ELF_header->e_ident[EI_MAGIC_1]
#define ELF_MAGIC_2	'L'  // ELF_header->e_ident[EI_MAGIC_2]
#define ELF_MAGIC_3	'F'  // ELF_header->e_ident[EI_MAGIC_3]
 
#define ELF_DATA_LSB	    (1)  // Little Endian
#define ELF_CLASS_32_BITS	(1)  // 32-bit Architecture

/*
 * ELF File type
 */
enum Elf_Filetype {
	ET_NONE		        = 0, // Unkown Type
	ET_RELOCATABLE		= 1, // Relocatable File
	ET_EXECUTABLE     = 2  // Executable File
};

#define ELF_MACHINE_386      (3)  // x86 Machine Type
#define ELF_CURRENT_VERSION	 (1)  // ELF Current Version

/*
 * Header of ELF files, present at the beginning
 * it provides important information about the file 
 * (such as the machine type, architecture and byte order, etc.)
 * as well as a means of identifying and checking whether the file is valid
 */
typedef struct {
  uint8_t		    ident[ELF_NIDENT];
	ELF32_Half	  type;               // Elf_Filetype
	ELF32_Half	  machine;            // ELF_MACHINE_386
	ELF32_Word	  version;            // 
	ELF32_Address	entry;
	ELF32_Offset	physOffset;
	ELF32_Offset	sectionHeaderOffset; // Position of the first section header from the beginning of the file img
	ELF32_Word    flags;
	ELF32_Half    ehSize;
	ELF32_Half    phEntrySize;
	ELF32_Half    phnum;
	ELF32_Half    sectionHeaderEntSize;
	ELF32_Half    sectionHeaderNum;
	ELF32_Half    sectionHeaderStrNdx;           // Index of the section name string table
} ELF32_header;
// -----------------------------------------------------------------------------------------------

# define ELF_SECTION_HEADER_NAMES_UNDEF	(0x00) // String section names table Undefined/Not present
 
/*
 * Types of sections
 */
enum SectionHeader_Types {
	SHT_NULL	    = 0,   // Null section
	SHT_PROGBITS	= 1,   // Program information
	SHT_SYMTAB	  = 2,   // Symbol table
	SHT_STRTAB	  = 3,   // String table
	SHT_RELA	    = 4,   // Relocation (w/ addend)
	SHT_NOBITS	  = 8,   // Not present in file
	SHT_REL		    = 9,   // Relocation (no addend)
};
 
enum SectionHeader_Attributes {
	SHF_WRITE	= 0x01, // Writable section
	SHF_ALLOC	= 0x02  // Exists in memory
};

/*
 * Section header contains information such as section:
 * - Names
 * - Sizes
 * - Locations
 * - Other relevant information
 */
typedef struct {
  ELF32_Word	  name;               // Offset within the ELF32_header->sectionHeaderStrNdx table
	ELF32_Word	  type;               // SectionHeader_Types
	ELF32_Word	  flags;              // SectionHeader_Attributes
	ELF32_Address	address;
	ELF32_Offset	offset;             // Position of the actual section from the beginning of the file img
	ELF32_Word	  size;
	ELF32_Word	  link;
	ELF32_Word	  info;
	ELF32_Word	  addressAlign;
	ELF32_Word	  entrySize;
} ELF32_sectionHeader;
// -----------------------------------------------------------------------------------------------


/*
 * Check if the given ELF header satisfy the magic signature
 */
bool elfCheckFile(ELF32_header *header);

/*
 * Check if the given ELF file is supported for the current machine
 * For our case we'll ensure that the ELF file is intended for a
 * i386, little endian and 32 bits machine
 */
bool elfCheckSupported(ELF32_header *header);

/*
 * Load the given ELF file
 */
void *elfLoadFile(void *file);

/*
 * Get the first section header (aka the index of the section headers) of the ELF file
 */
ELF32_sectionHeader *elfGetSectionHeaderNdx(ELF32_header *header);

/*
 * Get the indicated section header of the ELF file
 */
ELF32_sectionHeader *elfGetSectionHeader(ELF32_header *header, int index);

/*
 * Get the index of the section header names table
 */
char *elfGetSectionHeaderNamesTable (ELF32_header *header);

/*
 * Get the name of the given section
 */
char *elfGetSectionHeaderName (ELF32_header *header, ELF32_sectionHeader *sectionHeader);

