#include "fonts/terminus.h"
#include "psf.h"
#include "framebuffer.h"
#include <stddef.h>

uint16_t charIndexArray[128];
uint8_t unicodeTable = 0;
psf1_header* hdr = (psf1_header*) psf_font;

//Decodes the Unicode table (ONLY for ASCII codepoints)
void psfDecodeASCII()
{
    uint16_t* tbl = (uint16_t*)(psf_font + sizeof(psf1_header) + hdr->charsize * 256);

    uint16_t glyph = 0;
    while ((void*)tbl < (void*)psf_font + sizeof(psf_font))
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

void psfRender(uint32_t x, uint32_t y, uint16_t c, uint32_t bg, uint32_t fg)
{
    if(unicodeTable != 0)
        c = charIndexArray[c];

    uint8_t* glyph = psf_font + sizeof(psf1_header) + 
        (c>0 && c<256 ? c : 0) * hdr->charsize;

    framebufferDrawMonochromeBitmap(glyph, x, y, 8, hdr->charsize, bg, fg);
}

void psfGetCharSize(uint32_t* width, uint32_t* height)
{
    *width = 8;
    *height = hdr->charsize;
}