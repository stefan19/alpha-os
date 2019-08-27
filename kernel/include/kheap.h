#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HEADERSIZE ((sizeof(BLKHEADER_t) + 15) / 16) * 16

typedef struct HEAPHEADER
{
    uint32_t    vaddr;
    size_t      size;
    size_t      PAVaddr;
} HEAPHEADER_t;

typedef struct
{
    size_t  size;
    bool    used;
    void*   next;
    void*   prev;    
} BLKHEADER_t;

void* kheapAlloc(size_t size);
void* kheapAlloc_a(size_t n);
void kheapFree(void* address);
void kheapInit(uint32_t vaddr, uint32_t PAVaddr, size_t npages);

#endif