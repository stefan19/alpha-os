#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

/* enum VGA_COLOR
{
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_PURPLE,
    VGA_BROWN,
    VGA_GRAY,
    VGA_DARK_GRAY,
    VGA_LIGHT_BLUE,
    VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN,
    VGA_LIGHT_RED,
    VGA_LIGHT_PURPLE,
    VGA_YELLOW,
    VGA_WHITE,
};

inline uint8_t vgaColor(enum VGA_COLOR fg, enum VGA_COLOR bg)
{
    return fg | (bg << 4);
} */

void consoleInit(uint32_t bg, uint32_t fg);
void consolePutch(char c);
void consolePutchColor(char c, uint32_t color);
void consoleWriteStr(const char* str);
void consoleWriteStrColor(const char* str, uint32_t color);
void consoleWriteDec(uint32_t x);
void consoleWriteHex(uint32_t x);
void consoleClr(void);

#endif