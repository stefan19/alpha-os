#include <stddef.h>

#include "multiboot.h"
#include "framebuffer.h"
#include "psf.h"
#include "string.h"
#include "vmem.h"
#include "elf.h"
#include "membitmap.h"

extern uint32_t loader_end;
extern void jump_kernel(uint32_t entry, uint32_t bitmapAddr, uint32_t bitmapSize, uint32_t mbi, uint32_t loader_end);

void loader_exit(uint32_t entry, uint32_t newBitmapAddr)
{
    // Unmap the bitmap + mboot structure
    for(uint32_t page = memBitmapGetAddr(); page < memBitmapGetAddr() + memBitmapGetTotalSize(); page += 0x1000)
    {
        unmap(page);
    }

    uint32_t bitmapSize = memBitmapGetPhysMemSize() >> 15;
    uint32_t mbiAddr = newBitmapAddr + bitmapSize;
    mbiAddr = ((mbiAddr + 7) / 8) * 8;

    jump_kernel(entry, newBitmapAddr, bitmapSize, mbiAddr, (uint32_t)&loader_end);
}

void loader_main(multiboot_info_t* mbi)
{
    uint32_t tagaddr = (((uint32_t)mbi + sizeof(multiboot_info_t) + 7) / 8) * 8;

    mtag_generic_t* tag = (mtag_generic_t*) tagaddr;
    mtag_framebuf_t* fbuf = NULL;
    mtag_mods_t* kernel_mod = NULL;
    mtag_mmap_t* mmap_tag = NULL;

    while(tag->type != MBOOT_INFO_END)
    {
        if(tag->type == MBOOT_INFO_FRAMEBUF)
        {
            fbuf = (mtag_framebuf_t*) tag;
        }
        if(tag->type == MBOOT_INFO_MODS)
        {
            char* name = (char*)tag + offsetof(mtag_mods_t, string);
            if(strcmp(name, "kernel") == 0)
                kernel_mod = (mtag_mods_t*)tag;
        }
        if(tag->type == MBOOT_INFO_MMAP)
        {
            mmap_tag = (mtag_mmap_t*) tag;
        }

        tagaddr = ((tagaddr + tag->size + 7) / 8) * 8;
        tag = (mtag_generic_t*) tagaddr;
    }

    if(fbuf == NULL)
        return;
    if(fbuf->framebuffer_type != MBOOT_FRAMEBUF_DIRECT)
        return;
    if(kernel_mod == NULL)
        return;
    if(mmap_tag == NULL)
        return;

    uint32_t newBitmapAddr = vmemInit(kernel_mod, mmap_tag, fbuf, mbi);
    
    multiboot_info_t* new_mbi = memBitmapGetMBI();
    kernel_mod = (mtag_mods_t*)((void*)kernel_mod - (void*)mbi + (void*)new_mbi);

    uint32_t entry = elfLoadFromMem((void*)kernel_mod->mod_start);
    if(entry == 0)
        return;

    // The memory of the kernel image can be freed
    for(uint32_t i = kernel_mod->mod_start; i < kernel_mod->mod_end; i += 0x1000)
    {
        clearFrame(i);
        unmap(i);
    }
    
    loader_exit(entry, newBitmapAddr);
}