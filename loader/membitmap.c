#include "membitmap.h"
#include "string.h"
#include <stddef.h>

#define INDEX(a) (a/32)
#define OFFSET(a) (a%32)

uint32_t* bitmap;
uint32_t physicalMemSize = 0;
uint32_t bitmapSize;

extern uint32_t loader_end;
uint32_t placementAddr = (uint32_t)&loader_end;

static int regionsExclude(uint32_t r1_start, uint32_t r1_len, uint32_t r2_start, uint32_t r2_end) 
{
    return r1_start + r1_len <= r2_start || r1_start >= r2_end;
}

void setFrame(uint32_t addr)
{
    uint32_t page = addr >> 12;
    uint32_t idx = INDEX(page);
    uint32_t off = OFFSET(page);
    bitmap[idx] |= (1 << off);
}

void clearFrame(uint32_t addr)
{
    uint32_t page = addr >> 12;
    uint32_t idx = INDEX(page);
    uint32_t off = OFFSET(page);
    bitmap[idx] &= ~(1 << off);
}

uint8_t testFrame(uint32_t addr)
{
    uint32_t page = addr >> 12;
    uint32_t idx = INDEX(page);
    uint32_t off = OFFSET(page);
    return (bitmap[idx] >> off) & 0x1;
}

uint32_t freeFrame()
{
    uint32_t i, j;
    for(i=0; i<physicalMemSize >> 12; i++)
    {
        if(bitmap[i] != 0xFFFFFFFF)
        {
            for(j=0;j<32;j++)
            {
                uint32_t toTest = 1 << j;
                if( !(bitmap[i]&toTest) )
                {
                    return i*32 + j;
                }
            }
        }
    }
    return -1;
}

static void memBitmapMarkUnusable(mtag_mmap_t* mmap)
{
    uint32_t entry_addr = (uint32_t)mmap + offsetof(mtag_mmap_t, entries);

    while (entry_addr < (uint32_t)mmap + mmap->size)
    {
        mmap_entry_t* entry = (mmap_entry_t*) entry_addr;
        if (entry->type != MBOOT_MEM_AVAILABLE)
        {
            uint32_t start_addr = (entry->base_addr / 0x1000) * 0x1000;
            uint32_t end_addr = ((entry->base_addr + entry->length - 1) / 0x1000) * 0x1000;
            end_addr = end_addr >= physicalMemSize ? physicalMemSize : end_addr;

            for (uint32_t page = start_addr; page <= end_addr; page += 0x1000)
            {
                setFrame(page);
            }
        }

        entry_addr += mmap->entry_size;
    }
}

static void memBitmapPrepare(mtag_mmap_t* mmap) {
    uint32_t entry_addr = (uint32_t)mmap + offsetof(mtag_mmap_t, entries);

    while (entry_addr < (uint32_t)mmap + mmap->size)
    {
        mmap_entry_t* entry = (mmap_entry_t*) entry_addr;
        if (entry->type == MBOOT_MEM_AVAILABLE)
        {
            if (entry->base_addr + entry->length > physicalMemSize)
                physicalMemSize = entry->base_addr + entry->length;
        }

        entry_addr += mmap->entry_size;
    }
}

uint32_t memBitmapGetPhysMemSize()
{
    return physicalMemSize;
}

uint32_t memBitmapGetTotalSize()
{
    return bitmapSize;
}

uint32_t memBitmapGetAddr()
{
    return (uint32_t)bitmap;
}

multiboot_info_t* memBitmapGetMBI()
{
    uint32_t mbiAddr = (void*)bitmap + (physicalMemSize >> 15);
    mbiAddr = ((mbiAddr + 7) / 8) * 8;
    return (multiboot_info_t*)mbiAddr;
}

// Find a physical region to store the memory bitmap (which will be used in the kernel-proper)
// Avoid overwriting the kernel (which is a module) and the multiboot information structure
uint32_t memBitmapAllocate(mtag_mods_t* kernel_mod, mtag_mmap_t* mmap, multiboot_info_t* mbi)
{
    memBitmapPrepare(mmap);

    placementAddr = ((placementAddr+0x1000-1)/0x1000) * 0x1000;
    bitmapSize = (physicalMemSize >> 12) / 8 + mbi->total_size + 8;

    if (!regionsExclude(placementAddr, bitmapSize, kernel_mod->mod_start, kernel_mod->mod_end))
    {
        placementAddr = kernel_mod->mod_end;
    }

    placementAddr = ((placementAddr+0x1000-1)/0x1000) * 0x1000;
    bitmap = (uint32_t*) placementAddr;
    uint32_t mbiAddr = placementAddr + (physicalMemSize >> 12) / 8;
    mbiAddr = ((mbiAddr + 7) / 8) * 8;
    memmove((void*)mbiAddr, mbi, mbi->total_size);

    memBitmapMarkUnusable(mmap);

    for(uint32_t page = 0; page < 0x100000; page += 0x1000)
        setFrame(page);

    // Mark the module which contains the kernel as unusable memory
    for (uint32_t page = kernel_mod->mod_start; page < kernel_mod->mod_end; page += 0x1000)
        setFrame(page);

    // Mark the bitmap + multiboot structure as unusable memory
    for (uint32_t page = (uint32_t)bitmap; page < (uint32_t)bitmap + bitmapSize; page += 0x1000)
        setFrame(page);

    // Mark the loader as unusable memory
    uint32_t end_addr = (((uint32_t)&loader_end - 1) / 0x1000) * 0x1000;
    for (uint32_t page = 0x100000; page <= end_addr; page += 0x1000)
        setFrame(page);

    return (uint32_t) bitmap;
}