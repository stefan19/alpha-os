#include "gdt.h"
#include "console.h"
#include "string.h"

#define ENTRIES_COUNT 6

struct GDT_entry entries[ENTRIES_COUNT];
uint8_t GDT[ENTRIES_COUNT*8];

tss_entry_t tss_entry;

extern void gdtLoad(uint32_t base, uint16_t size);
extern void tssLoad();

void gdtEncodeEntry(uint8_t* target, struct GDT_entry entry)
{
    if(entry.limit > 65536) {
        entry.limit = entry.limit >> 12;
        target[6] = 0xC0;
    } else {
        target[6] = 0x40;
    }

    //Limit
    target[0] = entry.limit & 0xFF;
    target[1] = (entry.limit >> 8) & 0xFF;
    target[6] |= (entry.limit >> 16) & 0xF;

    //Base
    target[2] = entry.base & 0xFF;
    target[3] = (entry.base >> 8) & 0xFF;
    target[4] = (entry.base >> 16) & 0xFF;
    target[7] = (entry.base >> 24) & 0xFF;

    target[5] = entry.type;
}

void writeTSS(uint32_t num, uint32_t esp0, uint16_t ss0)
{
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry);

    memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.esp0 = esp0;
    tss_entry.ss0 = ss0;

    /* tss_entry.cs = 0x0b;        //Code segment selector for kernel | 0x3
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13; */

    entries[num] = (struct GDT_entry){.base = base, .limit = limit, .type = 0xE9};
}

void tssChangeESP0(uint32_t esp0)
{
    tss_entry.esp0 = esp0;
}

void gdtInit()
{
    entries[0] = (struct GDT_entry){.base = 0, .limit = 0, .type = 0};             //Null descriptor
    entries[1] = (struct GDT_entry){.base = 0, .limit = 0xFFFFFFFF, .type = 0x9A};    //Code descriptor
    entries[2] = (struct GDT_entry){.base = 0, .limit = 0xFFFFFFFF, .type = 0x92};    //Data descriptor
    entries[3] = (struct GDT_entry){.base = 0, .limit = 0xFFFFFFFF, .type = 0xFA};    //User code descriptor
    entries[4] = (struct GDT_entry){.base = 0, .limit = 0xFFFFFFFF, .type = 0xF2};    //User data descriptor

    writeTSS(5, 0, 0x10);       //Create a TSS for switching to ring 0

    for(uint8_t i=0;i<ENTRIES_COUNT;i++)
    {
        gdtEncodeEntry(GDT+i*8, entries[i]);
    }

    gdtLoad((uint32_t)&GDT, sizeof(GDT)-1);
    tssLoad();
}

