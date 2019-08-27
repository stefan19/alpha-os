#include "base_font.h"
#include "psf.h"
#include "framebuffer.h"
#include <stddef.h>

uint16_t charIndexArray[128];
uint8_t unicodeTable = 0;
psf1_header* hdr = (psf1_header*) base_font;

//Decodes the Unicode table (ONLY for ASCII codepoints)
void psfDecodeASCII()
{
    uint16_t* tbl = (uint16_t*)(base_font + sizeof(psf1_header) + hdr->charsize * 256);

    uint16_t glyph = 0;
    while ((void*)tbl < (void*)base_font + sizeof(base_font))
    {
        if(*tbl != 0xFFFF)
        {
            if(*tbl <= 127)
            {
                charIndexArray[*tbl] = glyph;
            }
        }
        else
        {
            glyph++;
        }

        tbl++;
    }
}

void psfOpen()
{
    if(hdr->magic[0] != PSF1_MAGIC0 || hdr->magic[1] != PSF1_MAGIC1)
        return;

    if(hdr->mode == PSF1_MODEHASTAB)
    {
        psfDecodeASCII();
        unicodeTable = 1;
    }
        
}

void psfRender(uint32_t x, uint32_t y, uint16_t c)
{
    if(unicodeTable != 0)
        c = charIndexArray[c];

    uint8_t* glyph = base_font + sizeof(psf1_header) + 
        (c>0 && c<256 ? c : 0) * hdr->charsize;

    size_t i, j, mask;
    for(j=0;j<hdr->charsize;j++)
    {
        mask = 1 << 7;
        for(i=0;i<8;i++)
        {
            if(*glyph & mask)
                framebufferPlotPixel(x+i, y+j, 0x303030);
            else
                framebufferPlotPixel(x+i, y+j, 0xA0A0A0);
            mask >>= 1;
        }

        glyph += 1;
    }
}

uint32_t x_pos, y_pos;

void psfRenderText(const char* text, mtag_framebuf_t* fbuf)
{
    while(*text != '\0')
    {
        psfRender(x_pos, y_pos, *text);
        text++;
        x_pos += 8;
        if(x_pos >= fbuf->framebuffer_width)
        {
            y_pos++;
            x_pos = 0;
        }
    }
}

void psfSetPenPos(uint32_t x, uint32_t y)
{
    x_pos = x;
    y_pos = y;
}