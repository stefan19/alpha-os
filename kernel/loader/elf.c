#include "elf.h"
#include "vfs.h"
#include "paging.h"
#include "task.h"

void elfLoadFromMem(void* ptr)
{
    elf_header* hdr = (elf_header*) ptr;
    if(hdr->elf_magic[0] != 0x7F || hdr->elf_magic[1] != 'E' ||
        hdr->elf_magic[2] != 'L' || hdr->elf_magic[3] != 'F')
    {
        printf("\nMalformed program file.");
        return;
    }

    if(hdr->elf_class != 1)
    {
        printf("\nCannot execute 64-bit program.");
        return;
    }

    if(hdr->elf_class != EARCH_x86)
    {
        printf("\nArchitecture of program incompatible.");
        return;
    }

    if(hdr->elf_objtype != ET_EXEC)
    {
        printf("\nObject file cannot be executed.");
        return;
    }

    printf("\nSize of program header entry: %u", sizeof(program_header));
    printf("\nNumber of program header entries: %u", hdr->elf_phnum);
}

int elfLoadSegment(vnode* file, program_header* ph)
{
    uint32_t first_page = ph->p_vaddr >> 12;
    uint32_t last_page = (ph->p_vaddr + ph->p_memsz - 1) >> 12;

    //Allocate the needed pages
    for(uint32_t k=first_page;k<=last_page;k++)
    {
        requestPage(k << 12);
    }

    return file->read(file, ph->p_offset, ph->p_filesz, (void*)ph->p_vaddr);
}

int elfLoadFromFile(const char* path, uint32_t* entry_point)
{
    vnode* file = vfsOpen(path);
    if(file == NULL)
    {
        printf("\nFile not found: %s", path);
        return -1;
    }

    uint8_t ptr[512];
    file->read(file, 0, 512, ptr);

    elf_header* hdr = (elf_header*) ptr;
    if(hdr->elf_magic[0] != 0x7F || hdr->elf_magic[1] != 'E' ||
        hdr->elf_magic[2] != 'L' || hdr->elf_magic[3] != 'F')
    {
        printf("\nMalformed program file.");
        return -1;
    }

    if(hdr->elf_class != 1)
    {
        printf("\nCannot execute 64-bit program.");
        return -1;
    }

    if(hdr->elf_machine != EARCH_x86)
    {
        printf("\nArchitecture of program incompatible.");
        return -1;
    }

    printf("\nSize of program header entry: %u", sizeof(program_header));
    printf("\nNumber of program header entries: %u", hdr->elf_phnum);
    printf("\nProgram entry: %x", hdr->elf_entry);

    program_header* ph = (program_header*) (ptr + hdr->elf_phoff);

    for(size_t i=0;i<hdr->elf_phnum;i++)
    {
        if(ph->p_type == PT_LOAD)
        {
            printf("\nLoadable section: %u, virtual address: %x", i, ph->p_vaddr);

            if(elfLoadSegment(file, ph) != 0)
                return -1;
        }
        ph++;
    }

    printf("\nProgram loaded: %s", path);
    *entry_point = hdr->elf_entry;
    return 0;
}