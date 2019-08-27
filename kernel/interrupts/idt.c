#include "idt.h"
#include "ll_io.h"

struct IDTDescr IDT[256];

//Exception handlers
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

//IRQ handlers
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

extern void syscall_handler();

void idtEncodeEntry(int no, uint32_t offset, uint16_t selector, uint8_t type_attr)
{
    IDT[no].zero = 0;

    //Offset
    IDT[no].offset_lo = offset & 0xFFFF;
    IDT[no].offset_hi = (offset >> 16) & 0xFFFF;

    //Selector
    IDT[no].selector = selector;

    //Attribute
    IDT[no].type_attr = type_attr;
}

void idtInit(void)
{
    //Filling the IDT
    idtEncodeEntry(0, (uint32_t)isr0, 0x08, 0x8E);
    idtEncodeEntry(1, (uint32_t)isr1, 0x08, 0x8E);
    idtEncodeEntry(2, (uint32_t)isr2, 0x08, 0x8E);
    idtEncodeEntry(3, (uint32_t)isr3, 0x08, 0x8E);
    idtEncodeEntry(4, (uint32_t)isr4, 0x08, 0x8E);
    idtEncodeEntry(5, (uint32_t)isr5, 0x08, 0x8E);
    idtEncodeEntry(6, (uint32_t)isr6, 0x08, 0x8E);
    idtEncodeEntry(7, (uint32_t)isr7, 0x08, 0x8E);
    idtEncodeEntry(8, (uint32_t)isr8, 0x08, 0x8E);
    idtEncodeEntry(9, (uint32_t)isr9, 0x08, 0x8E);
    idtEncodeEntry(10, (uint32_t)isr10, 0x08, 0x8E);
    idtEncodeEntry(11, (uint32_t)isr11, 0x08, 0x8E);
    idtEncodeEntry(12, (uint32_t)isr12, 0x08, 0x8E);
    idtEncodeEntry(13, (uint32_t)isr13, 0x08, 0x8E);
    idtEncodeEntry(14, (uint32_t)isr14, 0x08, 0x8E);
    idtEncodeEntry(15, (uint32_t)isr15, 0x08, 0x8E);
    idtEncodeEntry(16, (uint32_t)isr16, 0x08, 0x8E);
    idtEncodeEntry(17, (uint32_t)isr17, 0x08, 0x8E);
    idtEncodeEntry(18, (uint32_t)isr18, 0x08, 0x8E);
    idtEncodeEntry(19, (uint32_t)isr19, 0x08, 0x8E);
    idtEncodeEntry(20, (uint32_t)isr20, 0x08, 0x8E);
    idtEncodeEntry(21, (uint32_t)isr21, 0x08, 0x8E);
    idtEncodeEntry(22, (uint32_t)isr22, 0x08, 0x8E);
    idtEncodeEntry(23, (uint32_t)isr23, 0x08, 0x8E);
    idtEncodeEntry(24, (uint32_t)isr24, 0x08, 0x8E);
    idtEncodeEntry(25, (uint32_t)isr25, 0x08, 0x8E);
    idtEncodeEntry(26, (uint32_t)isr26, 0x08, 0x8E);
    idtEncodeEntry(27, (uint32_t)isr27, 0x08, 0x8E);
    idtEncodeEntry(28, (uint32_t)isr28, 0x08, 0x8E);
    idtEncodeEntry(29, (uint32_t)isr29, 0x08, 0x8E);
    idtEncodeEntry(30, (uint32_t)isr30, 0x08, 0x8E);
    idtEncodeEntry(31, (uint32_t)isr31, 0x08, 0x8E);

    idtEncodeEntry(32, (uint32_t)isr32, 0x08, 0x8E);
    idtEncodeEntry(33, (uint32_t)isr33, 0x08, 0x8E);
    idtEncodeEntry(34, (uint32_t)isr34, 0x08, 0x8E);
    idtEncodeEntry(35, (uint32_t)isr35, 0x08, 0x8E);
    idtEncodeEntry(36, (uint32_t)isr36, 0x08, 0x8E);
    idtEncodeEntry(37, (uint32_t)isr37, 0x08, 0x8E);
    idtEncodeEntry(38, (uint32_t)isr38, 0x08, 0x8E);
    idtEncodeEntry(39, (uint32_t)isr39, 0x08, 0x8E);
    idtEncodeEntry(40, (uint32_t)isr40, 0x08, 0x8E);
    idtEncodeEntry(41, (uint32_t)isr41, 0x08, 0x8E);
    idtEncodeEntry(42, (uint32_t)isr42, 0x08, 0x8E);
    idtEncodeEntry(43, (uint32_t)isr43, 0x08, 0x8E);
    idtEncodeEntry(44, (uint32_t)isr44, 0x08, 0x8E);
    idtEncodeEntry(45, (uint32_t)isr45, 0x08, 0x8E);
    idtEncodeEntry(46, (uint32_t)isr46, 0x08, 0x8E);
    idtEncodeEntry(47, (uint32_t)isr47, 0x08, 0x8E);

    idtEncodeEntry(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);

    idtLoad((uint32_t)&IDT, sizeof(IDT) - 1);
}