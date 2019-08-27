#include "stdio.h"
#include "console.h"
#include <stdarg.h>

void putchar(int c)
{
    consolePutch((char)c);
}

void puts(const char* s)
{
    consoleWriteStr(s);
}

void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while(*fmt != '\0')
    {
        if(*fmt != '%')
        {
            putchar(*fmt);
        }
        else
        {
            char t = *++fmt;
            switch(t)
            {
                case 'u': consoleWriteDec(va_arg(args, uint32_t)); break;
                case 'x': consoleWriteHex(va_arg(args, uint32_t)); break;
                case 'c': putchar(va_arg(args, int)); break;
                case 's': puts(va_arg(args, const char*)); break;
                case '%': putchar('%'); break;
                default: putchar(t);
            }
        }
        
        fmt++;
    }
}