#include "elf.h"
#include "string.h"
#include "vmem.h"
#include "membitmap.h"
#include "psf.h"

void elfLoadSegment(void* ptr, program_header* ph)
{
    uint32_t first_page = ph->p_vaddr >> 12;
    uint32_t last_page = (ph->p_vaddr + ph->p_memsz - 1) >> 12;

    //Allocate the needed pages and copy the segment
    for(uint32_t k=first_page;k<=last_page;k++)
    {
        mmap(k << 12);
    }

    memcpy((void*)ph->p_vaddr, ptr+ph->p_offset, ph->p_filesz);
}

// Loads the an ELF executable and returns the entry point
uint32_t elfLoadFromMem(void* ptr)
{
    elf_header* hdr = (elf_header*) ptr;
    if(hdr->elf_magic[0] != 0x7F || hdr->elf_magic[1] != 'E' ||
        hdr->elf_magic[2] != 'L' || hdr->elf_magic[3] != 'F')
    {
        psfRender(30, 40, '!');
        psfRender(38, 40, 'E');
        psfRender(46, 40, 'L');
        psfRender(54, 40, 'F');
        return 0;
    }

    if(hdr->elf_class != 1)
    {
        psfRender(30, 40, 'x');
        psfRender(38, 40, '6');
        psfRender(46, 40, '4');
        return 0;
    }

    if(hdr->elf_machine != EARCH_x86)
    {
        psfRender(30, 40, 'A');
        psfRender(38, 40, 'R');
        psfRender(46, 40, 'C');
        psfRender(54, 40, 'H');
        return 0;
    }

    if(hdr->elf_objtype != ET_EXEC)
    {
        return 0;
    }

    program_header* ph = (program_header*) (ptr + hdr->elf_phoff);
    for(size_t i=0;i<hdr->elf_phnum;i++)
    {
        if(ph->p_type == PT_LOAD)
        {
            elfLoadSegment(ptr, ph);
        }
        ph++;
    }

    return hdr->elf_entry;
}