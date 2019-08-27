#include <stddef.h>

#include "multiboot.h"
#include "framebuffer.h"
#include "psf.h"
#include "string.h"
#include "vmem.h"
#include "elf.h"

char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

extern uint32_t loader_end;

void loader_main(multiboot_info_t* mbi)
{
    uint32_t tagaddr = (((uint32_t)mbi + sizeof(multiboot_info_t) + 7) / 8) * 8;

    mtag_generic_t* tag = (mtag_generic_t*) tagaddr;
    mtag_framebuf_t* fbuf = NULL;
    mtag_mods_t* kernel_mod = NULL;
    mtag_mmap_t* mmap = NULL;

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
        if(tag->type == MBOOT_INFO_MMAP)
        {
            mmap = (mtag_mmap_t*) tag;
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

    /* framebufferInit(fbuf);
    framebufferClear(0xA0A0A0);

    psfOpen();
    psfSetPenPos(30, 30);
    if(kernel_mod)
    {
        psfRenderText("Module found: ", fbuf);
        const char* name = (char*) kernel_mod + offsetof(mtag_mods_t, string);
        psfRenderText(name, fbuf);
    }

    char buffer[30];
    psfRenderText("Framebuffer address: ", fbuf);
    itoa ((uint32_t)fbuf->framebuffer_addr, buffer, 16);
    psfRenderText(buffer, fbuf); */

    /*psfSetPenPos(30, 44);
    psfRenderText("Bitmap address: ", fbuf);
    itoa (memBitmapAllocate(kernel_mod, mmap, mbi), buffer, 16);
    psfRenderText(buffer, fbuf);

    psfSetPenPos(30, 58);
    psfRenderText("Kernel module: ", fbuf);
    itoa (kernel_mod->mod_start, buffer, 16);
    psfRenderText(buffer, fbuf);
    psfRenderText(", ", fbuf);
    itoa (kernel_mod->mod_end, buffer, 16);
    psfRenderText(buffer, fbuf);

    psfSetPenPos(30, 72);
    psfRenderText("Multiboot structure: ", fbuf);
    itoa ((uint32_t)mbi, buffer, 16);
    psfRenderText(buffer, fbuf);
    psfRenderText(", ", fbuf);
    itoa ((uint32_t)mbi + mbi->total_size, buffer, 16);
    psfRenderText(buffer, fbuf);

    psfSetPenPos(30, 86);
    psfRenderText("Kernel end: ", fbuf);
    itoa ((uint32_t)&loader_end, buffer, 16);
    psfRenderText(buffer, fbuf);

    psfSetPenPos(30, 100);
    psfRenderText("First free frame: ", fbuf);
    itoa ((uint32_t)freeFrame()*0x1000, buffer, 16);
    psfRenderText(buffer, fbuf);
    
    const char* text = "The system is loading.";
    psfSetPenPos(fbuf->framebuffer_width / 2 - strlen(text) * 4, fbuf->framebuffer_height*3/4);
    psfRenderText(text, fbuf); */

    vmemInit(kernel_mod, mmap, fbuf, mbi);

    framebufferInit(fbuf);
    framebufferClear(0xA0A0A0);

    elfLoadFromMem((void*)kernel_mod->mod_start);

    for(;;);

    //psfRender(x, fbuf->framebuffer_height * 3 / 4, strlen(text) + '0');
    //x += 8;
    /* while(*text != '\0')
    {
        psfRender(x, fbuf->framebuffer_height * 3 / 4, *text);
        x += 8;
        text++;
    } */
}