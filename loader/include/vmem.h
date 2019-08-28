#ifndef VMEM_H
#define VMEM_H

#include <stdint.h>
#include "multiboot.h"

typedef struct pt_entry
{
    uint8_t present     : 1;
    uint8_t rw          : 1;
    uint8_t user        : 1;
    uint8_t wt          : 1;
    uint8_t cdisable    : 1;
    uint8_t accessed    : 1;
    uint8_t dirty       : 1;
    uint8_t unused      : 5;
    uint32_t frame      : 20;
}__attribute__((packed)) pt_entry_t;

int isPageMapped(uint32_t vaddr);
void mmap(uint32_t vaddr);
void unmap(uint32_t vaddr);
uint32_t getPhysAddr(uint32_t vaddr);
uint32_t vmemInit(mtag_mods_t* kernel_mod, mtag_mmap_t* mmap, mtag_framebuf_t* fbuf, multiboot_info_t* mbi);

#endif