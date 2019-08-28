#include "kheap.h"
#include "paging.h"
#include "console.h"

HEAPHEADER_t heap_header;
uint32_t heap_default_request;

void kheapInit(uint32_t vaddr, uint32_t PAVaddr, size_t npages)
{
    for(uint32_t i=0;i<npages;i++)
        requestPage(vaddr+i*0x1000);

    heap_header.vaddr = vaddr;
    heap_header.size = 0x1000 * npages;
    heap_header.PAVaddr = PAVaddr;

    BLKHEADER_t* blk = (BLKHEADER_t*)vaddr;
    blk->next = NULL;
    blk->prev = NULL;
    blk->size = 0x1000 * npages - HEADERSIZE;
    blk->used = false;

    heap_default_request = npages;
}

void* kheapAlloc(size_t size)
{
    size = ((size + 15) / 16) * 16;

    BLKHEADER_t* blk = (BLKHEADER_t*)heap_header.vaddr;
    BLKHEADER_t* last_blk;
    while(blk != NULL)
    {
        if(blk->used == false && blk->size >= size)
            break;
        last_blk = blk;
        blk = (BLKHEADER_t*)blk->next;
    }
    if(blk == NULL)
    {
        // Requesting memory is needed
        uint32_t minNo;
        if (!last_blk->used)
        {
            minNo = (size - last_blk->size - 1) / 0x1000 + 1;
            blk = last_blk;
            blk->size = last_blk->size;
        }
        else
        {
            minNo = (size + HEADERSIZE - 1) / 0x1000 + 1;    
            blk = (BLKHEADER_t*)(heap_header.vaddr + heap_header.size);
            requestPage(heap_header.vaddr + heap_header.size);
            last_blk->next = blk;
            blk->prev = last_blk;
            blk->size = 0;
        }
        printf("Requested %x additional pages", minNo);
            
        uint32_t requestSize = minNo > heap_default_request ? minNo : heap_default_request;
        uint32_t i = requestSize;
        
        while(i-- > 0)
        {
            requestPage(heap_header.vaddr + heap_header.size);
            heap_header.size += 0x1000;
        }
        
        blk->next = NULL;
        blk->size += requestSize * 0x1000;
    }

    //blk is the first available block of requested size
    blk->used = true;

    //If there is more space available, split the block
    if(blk->size - size > HEADERSIZE + 32)
    {
        uint32_t end_addr = (uint32_t)blk + HEADERSIZE + size;
        uint32_t initial_size = blk->size;
        BLKHEADER_t* next_blk = (BLKHEADER_t*)blk->next;
        
        blk->size = size;
        blk->next = (void*)end_addr;

        BLKHEADER_t* new_blk = (BLKHEADER_t*)blk->next;
        new_blk->used = false;
        new_blk->next = (void*)next_blk;
        new_blk->prev = (void*)blk;
        new_blk->size = initial_size - size - HEADERSIZE;

        if(next_blk != NULL) next_blk->prev = (void*)new_blk;
    }

    return (void*)((uint32_t)blk + HEADERSIZE);
}

void kheapFree(void* address)
{
    BLKHEADER_t* blk = (BLKHEADER_t*)(address - HEADERSIZE);
    blk->used = false;

    //Merge this block with next free block if it exists
    BLKHEADER_t* next_blk = (BLKHEADER_t*)blk->next;
    if(next_blk != NULL && next_blk->used == false)
    {
        blk->next = next_blk->next;
        blk->size = blk->size + HEADERSIZE + next_blk->size;

        BLKHEADER_t* nn_block = (BLKHEADER_t*)next_blk->next;
        if(nn_block != NULL) nn_block->prev = (void*)blk;
    }

    //Merge this block with previous free block if it exists
    BLKHEADER_t* prev_blk = (BLKHEADER_t*)blk->prev;
    if(prev_blk != NULL && prev_blk->used == false)
    {
        prev_blk->next = blk->next;
        prev_blk->size = prev_blk->size + HEADERSIZE + blk->size;

        BLKHEADER_t* n_block = (BLKHEADER_t*)blk->next;
        if(n_block != NULL) n_block->prev = (void*)prev_blk;
    }

    return;
}

void* kheapAlloc_a(size_t n)
{
    uint32_t addr = heap_header.PAVaddr;
    for(size_t i=0;i<n;i++)
        requestPage(heap_header.PAVaddr + i * 0x1000);
    heap_header.PAVaddr += n * 0x1000;
    return addr;
}

void displayHeapStructure()
{
    BLKHEADER_t* blk = (BLKHEADER_t*)heap_header.vaddr;
    while(blk != NULL)
    {
        consoleWriteStr("\nBlock: ");
        consoleWriteHex((uint32_t)blk);
        consolePutch(' ');
        consoleWriteDec(blk->size);
        if(blk->used) consoleWriteStr(" used");
        else consoleWriteStr(" free");
        consolePutch(' ');
        consoleWriteHex((uint32_t)blk->prev);

        blk = (BLKHEADER_t*)blk->next;
    }
}