#include <stddef.h>
#include <stdbool.h>
#include <elf.h>

/*
 * TODO: change printf to ERROR 
 */

bool elfCheckFile(ELF32_header *header) {
  if (header->ident[EI_MAGIC_0] != ELF_MAGIC_0) {
    // printf("ELF header EI_MAGIC_0 incorrect.\n");
    return false;
  }
  if (header->ident[EI_MAGIC_1] != ELF_MAGIC_1) {
    // printf("ELF header EI_MAGIC_1 incorrect.\n");
    return false;
  }
  if (header->ident[EI_MAGIC_2] != ELF_MAGIC_2) {
    // printf("ELF header EI_MAGIC_2 incorrect.\n");
    return false;
  }
  if (header->ident[EI_MAGIC_3] != ELF_MAGIC_3) {
    // printf("ELF header EI_MAGIC_3 incorrect.\n");
    return false;
  }

  return true;
};

bool elfCheckSupported(ELF32_header *header) {
  if (!elfCheckFile(header)) {
    // printf("Invalid ELF file signature\n");
    return false;
  }
  if (header->ident[EI_CLASS] != ELF_CLASS_32_BITS) {
    // printf("Unsopported ELF file class (Supported is 32 bits)\n");
    return false;
  }
  if (header->ident[EI_DATA] != ELF_DATA_LSB) {
    // printf("Unsopported ELF file byte order (Supported is little endian)\n");
    return false;
  }
  if (header->ident[EI_VERSION] != ELF_CURRENT_VERSION) {
    // printf("Unsopported ELF file version (Supported is x86)\n");
    return false;
  }
  if (header->machine != ELF_MACHINE_386) {
    // printf("Unsopported ELF file target machine (Supported is x86)\n");
    return false;
  }
  if (header->type != ET_EXECUTABLE && header->type != ET_RELOCATABLE) {
    // printf("Unsopported ELF file type (Supported are executable and relocatable)\n");
    return false;
  }

  return true;
}

ELF32_sectionHeader *elfGetSectionHeaderNdx(ELF32_header *header) {
  return (ELF32_sectionHeader *) header + header->sectionHeaderOffset;
}

ELF32_sectionHeader *elfGetSectionHeader(ELF32_header *header, int index) {
  return &elfGetSectionHeaderNdx(header)[index];
}

char *elfGetSectionHeaderNamesTable (ELF32_header *header) {
  if (header->sectionHeaderStrNdx == ELF_SECTION_HEADER_NAMES_UNDEF) return NULL;
  return (char *)header + elfGetSectionHeader(header, header->sectionHeaderStrNdx)->offset;
}

char *elfGetSectionHeaderName (ELF32_header *header, ELF32_sectionHeader *sectionHeader) {
  // The section does not have a name
  if (sectionHeader->name == ELF_SECTION_HEADER_NAMES_UNDEF) return NULL;

  char *sectionHeaderNames = elfGetSectionHeaderNamesTable(header);
  if (sectionHeaderNames == NULL) return NULL;
  return sectionHeaderNames + sectionHeader->offset;
}

void *elfLoadFile(void *file) {
  ELF32_header *header = (ELF32_header *)file;

  if (!elfCheckSupported(header)) { 
    // printf("ELF file cannot be loaded\n");
    return NULL;
  };

  return NULL;
}
