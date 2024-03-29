#include <stddef.h>
#include "console.h"
#include "string.h"
#include "ll_io.h"

static uint8_t* const VGA_MEM = (uint8_t*)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t current_pos = 0;

void consoleEnableCursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void consoleDisableCursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void consoleUpdateCursor()
{
    uint16_t pos = current_pos / 2;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void consolePutch(char c)
{
    consolePutchColor(c, 0x7);
}

void consolePutchColor(char c, uint8_t color)
{
    if(c=='\n')
    {
        current_pos += 2*VGA_WIDTH - current_pos % (2*VGA_WIDTH); 
    }
    else if(c=='\b')
    {
        current_pos -= 2;
    }
    else
    {
        VGA_MEM[current_pos] = c;
        VGA_MEM[current_pos+1] = color;
        current_pos += 2;
    }

    consoleUpdateCursor();
}

void consoleWriteStr(const char* str)
{
    consoleWriteStrColor(str, 0x7);
}

void consoleWriteStrColor(const char* str, uint8_t color)
{
    while(*str != '\0')
    {
        if(*str=='\n')
        {
            current_pos += 2*VGA_WIDTH - current_pos % (2*VGA_WIDTH); 
        }
        else if(*str=='\b')
        {
            current_pos -= 2;
        }
        else
        {
            VGA_MEM[current_pos] = *str;
            VGA_MEM[current_pos+1] = color;
            current_pos += 2;
        }
        str++;
    }

    consoleUpdateCursor();
}

void consoleWriteDec(uint32_t x)
{
    char str[12];
    char* low;
    char* ptr;

    ptr = (char*)str;
    low = ptr;
    do
    {
        *ptr++ = x % 10 + '0';
        x /= 10;
    } while (x);
    *ptr-- = '\0';

    while(low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    
    consoleWriteStr(str);
}

void consoleWriteHex(uint32_t x)
{
    char str[12];
    char* low;
    char* ptr;

    ptr = (char*)str;
    low = ptr;
    do
    {
        *ptr++ = "0123456789ABCDEF"[x % 16];
        x /= 16;
    } while (x);
    *ptr-- = '\0';

    while(low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    
    consoleWriteStr(str);
}

void consoleWrite64(uint64_t x)
{
    char str[18];
    char* low;
    char* ptr;

    ptr = (char*)str;
    low = ptr;
    do
    {
        *ptr++ = "0123456789ABCDEF"[x % 16];
        x /= 16;
    } while (x);
    *ptr-- = '\0';

    while(low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    
    consoleWriteStr(str);
}

void consoleClr(void)
{
    consoleEnableCursor(14, 15);
    for(size_t i=0;i<2*VGA_HEIGHT*VGA_WIDTH;i+=2)
    {
        VGA_MEM[i] = ' ';
        VGA_MEM[i+1] = 0x7; 
    }
}

void consoleClrColor(uint8_t color)
{
    for(size_t i=0;i<2*VGA_HEIGHT*VGA_WIDTH;i+=2)
    {
        VGA_MEM[i] = ' ';
        VGA_MEM[i+1] = color; 
    }
}

void consolePanicScreen(const char* message)
{
    consoleClrColor(vgaColor(VGA_RED, VGA_BLACK));
    char title[] = "CRITICAL ERROR";

    current_pos = (80 * 5 + 40 - strlen(title) / 2) * 2;
    consoleWriteStrColor(title, vgaColor(VGA_RED, VGA_BLACK));

    current_pos = (80 * 8 + 15) * 2;
    consoleWriteStrColor(message, vgaColor(VGA_RED, VGA_BLACK));

    asm volatile("cli");
    asm volatile("hlt");
}