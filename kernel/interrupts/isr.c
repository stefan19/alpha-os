#include "isr.h"
#include "console.h"

isr_t interrupt_handlers[256];

void isrHandler(const interrupt_frame_t* frame)
{
    if(interrupt_handlers[frame->int_no] != 0)
    {
        isr_t handler = interrupt_handlers[frame->int_no];
        handler(frame);
    }
    else
    {
        consoleWriteStr("Unhandled interrupt: ");
        consoleWriteDec(frame->int_no);   
    }
}

void registerInterruptHandler(int no, isr_t handler)
{
    interrupt_handlers[no] = handler;
}