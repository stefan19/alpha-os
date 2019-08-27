#include "multiboot.h"
#include <stddef.h>

mtag_framebuf_t* fbuf = NULL;

void framebufferInit(mtag_framebuf_t* framebuf)
{
    if(framebuf->framebuffer_bpp != 24 && framebuf->framebuffer_bpp != 32)
        return;
    if(framebuf->framebuffer_red_mask_size != 8 || framebuf->framebuffer_green_mask_size != 8 ||
        framebuf->framebuffer_blue_mask_size != 8)
        return;
    fbuf = framebuf;
}

void framebufferPlotPixel(uint32_t x, uint32_t y, uint32_t color)
{
    if(fbuf == NULL)
        return;
    uint8_t* vmem = (uint8_t*)(0xE0000000 + fbuf->framebuffer_pitch * y + fbuf->framebuffer_bpp / 8 * x);

    vmem[fbuf->framebuffer_blue_field_pos/8] = color & 0xFF;
    vmem[fbuf->framebuffer_green_field_pos/8] = (color >> 8) & 0xFF;
    vmem[fbuf->framebuffer_red_field_pos/8] = (color >> 16) & 0xFF;
}

void framebufferClear(uint32_t color)
{
    uint8_t* vmem;

    size_t x, y;
    for(y=0; y < fbuf->framebuffer_height; y++)
    {
        vmem = (uint8_t*)(0xE0000000 + fbuf->framebuffer_pitch * y);

        for(x=0; x < fbuf->framebuffer_width; x++)
        {
            vmem[fbuf->framebuffer_blue_field_pos/8] = color & 0xFF;
            vmem[fbuf->framebuffer_green_field_pos/8] = (color >> 8) & 0xFF;
            vmem[fbuf->framebuffer_red_field_pos/8] = (color >> 16) & 0xFF;

            vmem += fbuf->framebuffer_bpp / 8;
        }
    }
}

void framebufferFillRct(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    uint8_t* vmem = (uint8_t*)(0xE0000000 + fbuf->framebuffer_pitch * y + fbuf->framebuffer_bpp / 8 * x);

    size_t i, j, idx;
    for(j=y;j<y+h;j++)
    {
        idx = 0;
        for(i=0; i<w; i++)
        {
            vmem[idx + fbuf->framebuffer_blue_field_pos/8] = color & 0xFF;
            vmem[idx + fbuf->framebuffer_green_field_pos/8] = (color >> 8) & 0xFF;
            vmem[idx + fbuf->framebuffer_red_field_pos/8] = (color >> 16) & 0xFF;

            idx += fbuf->framebuffer_bpp / 8;
        }

        vmem += fbuf->framebuffer_pitch;
    }
}