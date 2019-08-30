#include "paging.h"
#include "string.h"
#include "console.h"
#include "ll_io.h"

#define INDEX(a) (a/32)
#define OFFSET(a) (a%32)

uint32_t NFRAMES;
uint32_t* bitmap;
uint32_t* kernel_directory;

uint32_t master_directory[1024];

static void setFrame(uint32_t addr)
{
    uint32_t page = addr >> 12;
    uint32_t idx = INDEX(page);
    uint32_t off = OFFSET(page);
    bitmap[idx] |= (1 << off);
}

static void clearFrame(uint32_t addr)
{
    uint32_t page = addr >> 12;
    uint32_t idx = INDEX(page);
    uint32_t off = OFFSET(page);
    bitmap[idx] &= ~(1 << off);
}

static uint8_t testFrame(uint32_t addr)
{
    uint32_t page = addr >> 12;
    uint32_t idx = INDEX(page);
    uint32_t off = OFFSET(page);
    return (bitmap[idx] >> off) & 0x1;
}

static uint32_t freeFrame()
{
    uint32_t i, j;
    for(i=0; i<INDEX(NFRAMES); i++)
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

void initPaging(uint32_t bitmapAddr, uint32_t bitmapSize, uint32_t loaderEnd)
{
    bitmap = (uint32_t*) bitmapAddr;
    NFRAMES = bitmapSize << 3;
    printf("Number of available frames: %x", NFRAMES);
    printf("\nBitmap address: %x", (uint32_t)bitmap);

    for(uint32_t i=0x100000;i<loaderEnd;i+=0x1000)
    {
        unmap(i);
        clearFrame(i);
    }

    memcpy(master_directory, (void*)0xFFFFF000, 0x1000);
}

uint32_t getPhysAddress(uint32_t virtAddress)
{
    uint32_t pdindex = virtAddress >> 22;
    uint32_t ptindex = (virtAddress >> 12) & 0x3FF;

    if(master_directory[pdindex] == 0)
        return -1;
    
    uint32_t* pt = (uint32_t*) (0xFFC00000 + 0x1000 * pdindex);
    if(pt[ptindex] == 0)
        return -1;
    
    return (pt[ptindex] & ~0xFFF) + (virtAddress & 0xFFF); 
}

void mapVirtAddress(uint32_t virtAddress, uint32_t physAddress, uint8_t user)
{
    uint32_t pdindex = virtAddress >> 22;
    uint32_t ptindex = (virtAddress >> 12) & 0x3FF;

    if(master_directory[pdindex] == 0)
    {
        //Create page table if needed
        uint32_t table = freeFrame() * 0x1000;
        setFrame(table);
        master_directory[pdindex] = table | 0x7;

        memset((void*)(0xFFC00000 + 0x1000 * pdindex), 0, 0x1000);
    }

    pt_entry_t* pt = (pt_entry_t*) (0xFFC00000 + 0x1000 * pdindex);
    pt[ptindex].present = 1;
    pt[ptindex].rw = 1;
    pt[ptindex].user = user;
    pt[ptindex].frame = physAddress >> 12;

    invalidatePage(virtAddress);
}

void unmap(uint32_t vaddr)
{
    uint32_t pdindex = vaddr >> 22;
    uint32_t ptindex = (vaddr >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if (pd[pdindex] == 0)
        return;
    
    uint32_t* pt = (uint32_t*) (0xFFC00000 + 0x1000 * pdindex);
    pt[ptindex] = 0;

    invalidatePage(vaddr);
}

int checkPage(uint32_t virtAddress)
{
    uint32_t pdindex = virtAddress >> 22;
    uint32_t ptindex = (virtAddress >> 12) & 0x3FF;

    if(master_directory[pdindex] == 0)
        return 0;

    pt_entry_t* pt = (pt_entry_t*) (0xFFC00000 + 0x1000 * pdindex);
    if(pt[ptindex].present == 0)
        return 0;

    return 1;
}

void requestPage(uint32_t vaddr)
{
    if(checkPage(vaddr))
    {
        return;
    }
    
    uint32_t phys = freeFrame() * 0x1000;
    setFrame(phys);
    mapVirtAddress(vaddr, phys, 0);
}

void requestPageUser(uint32_t vaddr)
{
    if(checkPage(vaddr))
        return;

    uint32_t phys = freeFrame() * 0x1000;
    setFrame(phys);
    mapVirtAddress(vaddr, phys, 1);
}

void handlePageFault(uint32_t err_code)
{
    uint8_t present = err_code & 0x1;
    uint8_t user = err_code & 0x4;

    if (!present & !user)
    {
        // A non-present address was accessed
        uint32_t addr = readCR2();
        if (addr >= 0xFFC00000 && addr < 0xFFFFF000)
        {
            uint32_t pdindex = (addr - 0xFFC00000) >> 12;
            uint32_t* pd = (uint32_t*) 0xFFFFF000;
            pd[pdindex] = master_directory[pdindex];
            invalidatePage(0xFFC00000 + 0x1000 * pdindex);
        }
        else if (addr >= 0xC0000000 && addr < 0xFFC00000)
        {
            uint32_t pdindex = addr >> 22;
            uint32_t* pd = (uint32_t*) 0xFFFFF000;
            pd[pdindex] = master_directory[pdindex];
            invalidatePage(0xFFC00000 + 0x1000 * pdindex);
        }

        if(checkPage(addr))
        {
            printf("Resolved page fault at %x", addr);
            return;
        }
    }
    
    consoleWriteStrColor("FATAL ERROR: Unresolved page fault", 0xD00000);
    printf(" at address: %x", readCR2());
    asm volatile("cli");
    asm volatile("hlt");
}

uint32_t newAddressSpace()
{
    uint32_t* pd = (uint32_t*) (freeFrame()*0x1000);
    setFrame((uint32_t)pd);

    uint32_t* new_pd = (uint32_t*)0xEFFFF000;
    mapVirtAddress((uint32_t)new_pd, (uint32_t)pd, 0);

    // Copy the high address mappings (0xC0000000 - ...)
    uint32_t pdindex = 0xC0000000 >> 22;
    for(uint32_t i = pdindex; i < 0x3FF; i++)
    {
        new_pd[i] = master_directory[i];
    }
    new_pd[0x3FF] = (uint32_t) pd | 0x3;

    return (uint32_t) pd;
}