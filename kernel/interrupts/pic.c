#include "pic.h"
#include "ll_io.h"

#define PIC_MASTER 0x20
#define PIC_SLAVE 0xA0

void picInit(uint8_t port1, uint8_t port2)
{
    outb(PIC_MASTER, 0x11);
    outb(PIC_SLAVE, 0x11);

    //ICW2
    outb(PIC_MASTER+1, port1);
    outb(PIC_SLAVE+1, port2);

    //ICW3
    outb(PIC_MASTER+1, 4);
    outb(PIC_SLAVE+1, 2);

    //ICW4
    outb(PIC_MASTER+1, 1);
    outb(PIC_SLAVE+1, 1);

    outb(PIC_MASTER+1, 0xFF);
    outb(PIC_SLAVE+1, 0xFF);
}

void picMask(uint8_t intLine)
{
    uint16_t port;
    uint8_t value;

    if(intLine < 8)
        port = PIC_MASTER+1;
    else
    {
        port = PIC_SLAVE+1;
        intLine -= 8;
    }

    value = inb(port) | (1 << intLine);
    outb(port, value);
}

void picUnmask(uint8_t intLine)
{
    uint16_t port;
    uint8_t value;

    if(intLine < 8)
        port = PIC_MASTER+1;
    else
    {
        outb(PIC_MASTER+1, inb(PIC_MASTER+1) & (~4));
        port = PIC_SLAVE+1;
        intLine -= 8;
    }

    value = inb(port) & (~(1 << intLine));
    outb(port, value);
}

void picAck(uint8_t intLine)
{
    if(intLine >= 8)
        outb(PIC_SLAVE, 0x20);

    outb(PIC_MASTER, 0x20);
}