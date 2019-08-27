#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "multiboot.h"

void framebufferInit(mtag_framebuf_t* framebuf);
void framebufferPlotPixel(uint32_t x, uint32_t y, uint32_t color);
void framebufferClear(uint32_t color);
void framebufferFillRct(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

#endif