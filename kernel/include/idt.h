#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct IDTDescr {
    uint16_t offset_lo;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_hi;
} __attribute__((packed));

extern void idtLoad(uint32_t base, uint32_t limit);
void idtInit(void);

#endif