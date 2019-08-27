#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot.h"

typedef struct pt_entry
{
    uint8_t present     : 1;
    uint8_t rw          : 1;
    uint8_t user        : 1;
    uint8_t accessed    : 1;
    uint8_t dirty       : 1;
    uint8_t unused      : 7;
    uint32_t frame      : 20;
}__attribute__((packed)) pt_entry_t;

void initPagingStructures(multiboot_mmap_t*, uint32_t, uint32_t);
void initPaging(void);
extern void loadPageDirectory(uint32_t addr);
extern void enablePaging();
extern void invalidatePage(uint32_t addr);
uint32_t getPhysAddress(uint32_t virtAddress);
void mapVirtAddress(uint32_t virtAddress, uint32_t physAddress, uint8_t user);
void requestPage(uint32_t vaddr);
void requestPageUser(uint32_t vaddr);

#endif