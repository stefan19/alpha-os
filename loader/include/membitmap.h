#ifndef MEM_BITMAP_H
#define MEM_BITMAP_H

#include "multiboot.h"
#include <stdint.h>

void setFrame(uint32_t addr);
void clearFrame(uint32_t addr);
uint8_t testFrame(uint32_t addr);
uint32_t freeFrame();

uint32_t memBitmapGetPhysMemSize();
uint32_t memBitmapAllocate(mtag_mods_t* kernel_mod, mtag_mmap_t* mmap, multiboot_info_t* mbi);

#endif