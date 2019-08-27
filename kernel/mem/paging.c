#include "paging.h"
#include "string.h"
#include "console.h"

#define INDEX(a) (a/32)
#define OFFSET(a) (a%32)

uint32_t NFRAMES;
uint32_t* bitmap;
uint32_t* kernel_directory;

extern char end;
uint32_t placement_addr;

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

static uint32_t memAlloc(uint8_t align, uint32_t size)
{
    if(align == 1)
    {
        placement_addr = (placement_addr - 1) & 0xFFFFF000;
        placement_addr += 0x1000;
    }

    uint32_t tmp = placement_addr;
    placement_addr += size;
    return tmp;
}

void earlyMapPage(uint32_t addr, uint32_t physAddr)
{
    //Turn the address into an index
    addr = addr >> 12;
    uint32_t tbl_idx = addr >> 10;

    pt_entry_t* table = (pt_entry_t*)((kernel_directory[tbl_idx] & 0xFFFFF000) + 0xBFF00000);

    //Create table if needed
    if(kernel_directory[tbl_idx] == 0)
    {
        table = (pt_entry_t*)memAlloc(1, 0x1000);
        setFrame((uint32_t)table-0xBFF00000);
        memset(table, 0, 0x1000);

        kernel_directory[tbl_idx] = ((uint32_t)table-0xBFF00000) | 0x7;
    }

    //Create page entry
    setFrame(physAddr);

    table[addr & 0x3FF].frame = physAddr >> 12;
    table[addr & 0x3FF].present = 1;
    table[addr & 0x3FF].rw = 1;
    table[addr & 0x3FF].user = 0;
}

void initPagingStructures(multiboot_mmap_t* memoryMap, uint32_t mmapAddr, uint32_t mmapLength)
{
    multiboot_mmap_t* mmap = memoryMap;

    uint32_t usableMemEnd = 0;
    while((uint32_t)mmap < mmapAddr + mmapLength)
    {
        if(mmap->addr > usableMemEnd && mmap->type == 1)
            usableMemEnd = mmap->addr + mmap->len;
        /* consoleWriteStr("\nMemory: ");
        consoleWriteHex(mmap->addr);
        consolePutch(' ');
        consoleWriteHex(mmap->len);
        if(mmap->type == 1)
            consoleWriteStr(" usable"); */
        mmap = (multiboot_mmap_t*)( (uint32_t)mmap + mmap->size + sizeof(mmap->size)); 
    }

    NFRAMES = usableMemEnd / 0x1000;

    placement_addr = (uint32_t)&end;
    bitmap = (uint32_t*)memAlloc(0, NFRAMES/8);
    memset(bitmap, 0, NFRAMES/8);

    mmap = memoryMap;
    while((uint32_t)mmap < mmapAddr + mmapLength)
    {
        if(mmap->type != 1)
        {
            //Mark unusable RAM as such
            uint32_t startingFrame = mmap->addr / 0x1000;
            uint32_t endFrame = (mmap->addr + mmap->len - 1) / 0x1000 + 1;
            for(uint32_t i=startingFrame;i<endFrame && i<NFRAMES;i++)
            {
                setFrame(i*0x1000);
            }
        }
        mmap = (multiboot_mmap_t*)( (uint32_t)mmap + mmap->size + sizeof(mmap->size)); 
    }
}

void initPaging(void)
{
    //Create an empty page directory (kernel's)
    kernel_directory = (uint32_t*)memAlloc(1, 0x1000);
    memset(kernel_directory, 0, 0x1000);
    setFrame((uint32_t)kernel_directory-0xBFF00000);

    //Identity map the lowest 1 MiB
    for(uint32_t i=0;i<0x100000;i+=0x1000)
        earlyMapPage(i, i);

    //Map the kernel to 0xC0000000 (higher half)
    for(uint32_t i=0xC0000000;i<(uint32_t)kernel_directory;i+=0x1000)
        earlyMapPage(i, i - 0xBFF00000);

    //Map the last page table to the directory itself
    kernel_directory[0x3FF] = ((uint32_t)kernel_directory - 0xBFF00000) | 0x3;

    loadPageDirectory((uint32_t)kernel_directory-0xBFF00000);
}

uint32_t getPhysAddress(uint32_t virtAddress)
{
    uint32_t pdindex = virtAddress >> 22;
    uint32_t ptindex = (virtAddress >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if(pd[pdindex] == 0)
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
    pt[ptindex].frame = physAddress >> 12;

    invalidatePage(virtAddress);
}

int checkPage(uint32_t virtAddress)
{
    uint32_t pdindex = virtAddress >> 22;
    uint32_t ptindex = (virtAddress >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*) 0xFFFFF000;
    if(pd[pdindex] == 0)
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
        printf("Page already mapped");
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