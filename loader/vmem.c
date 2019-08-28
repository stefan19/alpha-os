#include "vmem.h"
#include "string.h"
#include "membitmap.h"

extern void enablePaging();
extern void switchPageDirectory(uint32_t pagedir);
extern void invalidatePage(uint32_t page);

uint32_t* page_directory;

extern uint32_t loader_end;

int isPageMapped(uint32_t vaddr)
{
    uint32_t pdindex = vaddr >> 22;
    uint32_t ptindex = (vaddr >> 12) & 0x3FF;
    
    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if(pd[pdindex] == 0)
        return 0;

    pt_entry_t* pt = (pt_entry_t*) (0xFFC00000 + 0x1000 * pdindex);
    if(pt[ptindex].present == 0)
        return 0;

    return 1;
}

void earlyMmap(uint32_t vaddr, uint32_t paddr)
{
    uint32_t pdindex = vaddr >> 22;
    uint32_t ptindex = (vaddr >> 12) & 0x3FF;

    if(page_directory[pdindex] == 0)
    {
        //Create page table if needed
        uint32_t table = freeFrame() * 0x1000;
        setFrame(table);
        page_directory[pdindex] = table | 0x7;

        memset((void*)table, 0, 0x1000);
    }

    pt_entry_t* pt = (pt_entry_t*)(page_directory[pdindex] & 0xFFFFF000);
    pt[ptindex].present = 1;
    pt[ptindex].rw = 1;
    pt[ptindex].user = 0;
    pt[ptindex].frame = paddr >> 12;
}

void mmap(uint32_t vaddr)
{
    if (isPageMapped(vaddr))
        return;

    uint32_t paddr = freeFrame() * 0x1000;
    setFrame(paddr);

    uint32_t pdindex = vaddr >> 22;
    uint32_t ptindex = (vaddr >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if(pd[pdindex] == 0)
    {
        //Create page table if needed
        uint32_t table = freeFrame() * 0x1000;
        setFrame(table);
        pd[pdindex] = table | 0x7;

        invalidatePage(0xFFC00000 + 0x1000 * pdindex);
        memset((void*)(0xFFC00000 + 0x1000 * pdindex), 0, 0x1000);
    }

    pt_entry_t* pt = (pt_entry_t*) (0xFFC00000 + 0x1000 * pdindex);
    pt[ptindex].present = 1;
    pt[ptindex].rw = 1;
    pt[ptindex].user = 0;
    pt[ptindex].frame = paddr >> 12;

    invalidatePage(vaddr);
}

void unmap(uint32_t vaddr)
{
    uint32_t pdindex = vaddr >> 22;
    uint32_t ptindex = (vaddr >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if(pd[pdindex] == 0)
        return;

    uint32_t* pt = (uint32_t*) (0xFFC00000 + 0x1000 * pdindex);
    pt[ptindex] = 0;
    
    invalidatePage(vaddr);
}

uint32_t getPhysAddr(uint32_t vaddr)
{
    uint32_t pdindex = vaddr >> 22;
    uint32_t ptindex = (vaddr >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if(pd[pdindex] == 0)
        return 0xFFFFFFFF;

    pt_entry_t* pt = (pt_entry_t*) (0xFFC00000 + 0x1000 * pdindex);
    if(pt[ptindex].present == 0)
        return 0xFFFFFFFF;

    return pt[ptindex].frame << 12;
}

uint32_t vmemInit(mtag_mods_t* kernel_mod, mtag_mmap_t* mmap, mtag_framebuf_t* fbuf, multiboot_info_t* mbi)
{
    uint32_t bitmapAddr = memBitmapAllocate(kernel_mod, mmap, mbi);
    multiboot_info_t* new_mbi = memBitmapGetMBI();
    kernel_mod = (mtag_mods_t*)((void*)kernel_mod - (void*)mbi + (void*)new_mbi);
    mmap = (mtag_mmap_t*)((void*)mmap - (void*)mbi + (void*)new_mbi);
    fbuf = (mtag_framebuf_t*)((void*)fbuf - (void*)mbi + (void*)new_mbi);

    //Create the page directory
    page_directory = (uint32_t*)(freeFrame() * 0x1000);
    setFrame((uint32_t)page_directory);
    memset(page_directory, 0, 0x1000);

    //Identity map the loader, bitmap, kernel module, multiboot structure
    for(uint32_t i = 0x100000; i < (uint32_t)&loader_end; i += 0x1000)
        earlyMmap(i, i);
    
    for (uint32_t page = kernel_mod->mod_start; page < kernel_mod->mod_end; page += 0x1000)
        earlyMmap(page, page);

    for (uint32_t page = bitmapAddr; page <= bitmapAddr + memBitmapGetTotalSize(); page += 0x1000)
        earlyMmap(page, page);

    // Map video memory to 0xE0000000
    for (uint32_t page = 0xE0000000; page <= 0xE0000000+fbuf->framebuffer_height*fbuf->framebuffer_pitch; page += 0x1000)
        earlyMmap(page, (uint32_t)fbuf->framebuffer_addr + page-0xE0000000);

    // Map the bitmap + multiboot structure
    uint32_t page_count = (memBitmapGetTotalSize()-1)/0x1000 + 1;
    uint32_t newBitmapAddr = 0xD0000000 - page_count*0x1000;

    for (uint32_t i = 0; i < page_count; i++)
    {
        earlyMmap(0xD0000000 - page_count*0x1000 + i*0x1000, bitmapAddr + i*0x1000);
    }

    // Map the last entry in the page directory to itself
    page_directory[0x3FF] = (uint32_t) page_directory | 0x3;
    
    switchPageDirectory((uint32_t)page_directory);
    enablePaging();

    return newBitmapAddr;
}