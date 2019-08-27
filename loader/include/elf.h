#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define ET_NONE     0x00
#define ET_REL      0x01
#define ET_EXEC     0x02
#define ET_DYN      0x03
#define ET_CORE     0x04
#define ET_LOOS     0xFE00
#define ET_HIOS     0xFEFF
#define ET_LOPROC   0xFF00
#define ET_HIPROC   0xFFFF

#define EARCH_NONE      0x00
#define EARCH_SPARC     0x02
#define EARCH_x86       0x03
#define EARCH_MIPS      0x08
#define EARCH_PPC       0x14
#define EARCH_S390      0x16
#define EARCH_ARM       0x28
#define EARCH_SUPERH    0x2A
#define EARCH_IA_64     0x32
#define EARCH_x86_64    0x3E
#define EARCH_AArch64   0xB7
#define EARCH_RISC_V    0xF3

#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_LOOS     0x60000000
#define PT_HIOS     0x6FFFFFFF
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7FFFFFFF

typedef struct
{
    uint8_t elf_magic[4];
    uint8_t elf_class;
    uint8_t elf_endian;
    uint8_t elf_idversion;
    uint8_t elf_abi;
    uint8_t elf_abiver;
    uint8_t elf_pad[7];
    uint16_t elf_objtype;
    uint16_t elf_machine;
    uint32_t elf_version;
    uint32_t elf_entry;
    uint32_t elf_phoff;
    uint32_t elf_shoff;
    uint32_t elf_flags;
    uint16_t elf_header_size;
    uint16_t elf_phentsize;
    uint16_t elf_phnum;
    uint16_t elf_shentsize;
    uint16_t elf_shnum;
    uint16_t elf_shstrndx;
}__attribute__((packed)) elf_header;

typedef struct
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
}__attribute__((packed)) program_header;

void elfLoadFromMem(void* ptr);

#endif