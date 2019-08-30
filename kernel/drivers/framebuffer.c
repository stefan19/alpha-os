#include "framebuffer.h"

mtag_framebuf_t* fbuf;

void framebufferInit(multiboot_info_t* mbi)
{
    uint32_t tagaddr = (((uint32_t)mbi + sizeof(multiboot_info_t) + 7) / 8) * 8;
    mtag_generic_t* tag = (mtag_generic_t*) tagaddr;

    while(tag->type != MBOOT_INFO_END)
    {
        if(tag->type == MBOOT_INFO_FRAMEBUF)
        {
            fbuf = (mtag_framebuf_t*) tag;
        }
        tagaddr = ((tagaddr + tag->size + 7) / 8) * 8;
        tag = (mtag_generic_t*) tagaddr;
    }
}

void framebufferClear(uint32_t color)
{
    uint8_t* vmem;

    uint32_t x, y;
    for(y=0; y < fbuf->framebuffer_height; y++)
    {
        vmem = (uint8_t*)(LINEAR_FRAMEBUF + fbuf->framebuffer_pitch * y);

        for(x=0; x < fbuf->framebuffer_width; x++)
        {
            vmem[fbuf->framebuffer_blue_field_pos/8] = color & 0xFF;
            vmem[fbuf->framebuffer_green_field_pos/8] = (color >> 8) & 0xFF;
            vmem[fbuf->framebuffer_red_field_pos/8] = (color >> 16) & 0xFF;

            vmem += fbuf->framebuffer_bpp / 8;
        }
    }
}

void framebufferDrawPixel(uint32_t x, uint32_t y, uint32_t color)
{
    uint8_t* where = (uint8_t*)(LINEAR_FRAMEBUF + y * fbuf->framebuffer_pitch + x * fbuf->framebuffer_bpp/8);
    where[fbuf->framebuffer_blue_field_pos/8] = color & 0xFF;
    where[fbuf->framebuffer_green_field_pos/8] = (color >> 8) & 0xFF;
    where[fbuf->framebuffer_red_field_pos/8] = (color >> 16) & 0xFF;
}

void framebufferFillRct(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    uint8_t* vmem = (uint8_t*)(LINEAR_FRAMEBUF + fbuf->framebuffer_pitch * y + fbuf->framebuffer_bpp / 8 * x);

    uint32_t i, j, idx;
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

void framebufferDrawMonochromeBitmap(uint8_t* bmp, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t bg, uint32_t fg)
{
    uint8_t* vmem;
    uint32_t i, j, q=0;

    for(j = y; j < y + h; j++)
    {
        vmem = (uint8_t*)(LINEAR_FRAMEBUF + fbuf->framebuffer_pitch * j + fbuf->framebuffer_bpp/8 * x);

        for(i = x; i < x + w; i++)
        {
            uint32_t color;
            if(bmp[q>>3] & (1 << (7 - (q&7))))
                color = fg;
            else
                color = bg;
            vmem[fbuf->framebuffer_blue_field_pos/8] = color & 0xFF;
            vmem[fbuf->framebuffer_green_field_pos/8] = (color >> 8) & 0xFF;
            vmem[fbuf->framebuffer_red_field_pos/8] = (color >> 16) & 0xFF;

            vmem += fbuf->framebuffer_bpp / 8;
            q++;
        }
    }
}

void framebufferGetSize(uint32_t* width, uint32_t* height)
{
    *width = fbuf->framebuffer_width;
    *height = fbuf->framebuffer_height;
}