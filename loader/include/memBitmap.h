#ifndef MEM_BITMAP_H
#define MEM_BITMAP_H

#include "multiboot.h"
#include <stdint.h>

void memBitmapPrepare(mtag_mmap_t* mmap);
uint32_t memBitmapGetPhysMemSize();
uint32_t memBitmapAllocate(mtag_mods_t* kernel_mod, mtag_mmap_t* mmap, multiboot_info_t* mbi);

#endif