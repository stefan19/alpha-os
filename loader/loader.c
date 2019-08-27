#include "multiboot.h"
#include "framebuffer.h"
#include "psf.h"
#include "string.h"
#include <stddef.h>

void loader_main(multiboot_info_t* mbi)
{
    uint32_t tagaddr = (((uint32_t)mbi + sizeof(multiboot_info_t) + 7) / 8) * 8;

    mtag_generic_t* tag = (mtag_generic_t*) tagaddr;
    mtag_framebuf_t* fbuf = NULL;
    mtag_mods_t* kernel_mod = NULL;

    while(tag->type != MBOOT_INFO_END)
    {
        if(tag->type == MBOOT_INFO_FRAMEBUF)
        {
            fbuf = (mtag_framebuf_t*) tag;
        }
        if(tag->type == MBOOT_INFO_MODS)
        {
            kernel_mod = (mtag_mods_t*) tag;
        }
        tagaddr = ((tagaddr + tag->size + 7) / 8) * 8;
        tag = (mtag_generic_t*) tagaddr;
    }

    if(fbuf == NULL)
    {
        return;
    }
        
    if(fbuf->framebuffer_type != MBOOT_FRAMEBUF_DIRECT)
    {
        return;
    }

    /* uint8_t* vram = (uint8_t*)fbuf->framebuffer_addr;

    for(size_t i=0;i<=fbuf->framebuffer_width*fbuf->framebuffer_height*fbuf->framebuffer_bpp/8;i+=4)
    {
        vram[i] = 0xee;
        vram[i+1] = 0x00;
        vram[i+2] = 0x00;
        vram[i+3] = 0;
    } */

    framebufferInit(fbuf);
    framebufferClear(0xA0A0A0);

    psfOpen();
    psfSetPenPos(30, 30);
    if(kernel_mod)
    {
        psfRenderText("Module found: ", fbuf);
        const char* name = (char*) kernel_mod + offsetof(mtag_mods_t, string);
        psfRenderText(name, fbuf);
    }
    
    const char* text = "The system is loading.";
    psfSetPenPos(fbuf->framebuffer_width / 2 - strlen(text) * 4, fbuf->framebuffer_height*3/4);
    psfRenderText(text, fbuf);

    //psfRender(x, fbuf->framebuffer_height * 3 / 4, strlen(text) + '0');
    //x += 8;
    /* while(*text != '\0')
    {
        psfRender(x, fbuf->framebuffer_height * 3 / 4, *text);
        x += 8;
        text++;
    } */
}