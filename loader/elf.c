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
        mmap(k);
    }

    memcpy((void*)ph->p_vaddr, ptr+ph->p_offset, ph->p_filesz);
}

void elfLoadFromMem(void* ptr)
{
    elf_header* hdr = (elf_header*) ptr;
    if(hdr->elf_magic[0] != 0x7F || hdr->elf_magic[1] != 'E' ||
        hdr->elf_magic[2] != 'L' || hdr->elf_magic[3] != 'F')
    {
        psfRender(30, 40, 'E');
        psfRender(38, 40, 'L');
        psfRender(46, 40, 'F');
        return;
    }

    if(hdr->elf_class != 1)
    {
        return;
    }

    if(hdr->elf_class != EARCH_x86)
    {
        return;
    }

    if(hdr->elf_objtype != ET_EXEC)
    {
        return;
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
}