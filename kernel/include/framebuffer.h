#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define LINEAR_FRAMEBUF 0xE0000000
#include "multiboot.h"

void framebufferInit(multiboot_info_t* mbi);
void framebufferClear(uint32_t color);
void framebufferDrawPixel(uint32_t x, uint32_t y, uint32_t color);
void framebufferDrawMonochromeBitmap(uint8_t* bmp, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t bg, uint32_t fg);
void framebufferGetSize(uint32_t* width, uint32_t* height);

#endif