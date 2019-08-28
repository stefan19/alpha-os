#include "isr.h"
#include "console.h"
#include "ll_io.h"

const char error_strings[21][87] = {
    "division by 0 error",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "overflow",
    "bound range exceeded",
    "invalid opcode",
    "floating point instruction can't be executed - the system lacks a floating point unit",
    "double fault",
    "?",
    "invalid TSS",
    "segment not present",
    "stack segment fault",
    "general protection fault",
    "page fault",
    "?",
    "x87 floating point exception",
    "alignment check",
    "machine check",
    "SIMD floating point exception",
    "virtualization exception",    
};

void exceptionHandler(const interrupt_frame_t* frame)
{
    consoleWriteStrColor("\nFATAL ERROR: ", 0xD00000);
    if(frame->int_no <= 20)
        consoleWriteStrColor(error_strings[frame->int_no], 0xD00000);   
    else if(frame->int_no == 30)
        consoleWriteStrColor("security exception", 0xD00000);
    else
        consoleWriteStrColor("?", 0xD00000);

    if(frame->int_no == 14)
    {
        consoleWriteStrColor(" at address: ", 0xD00000);
        consoleWriteHex(readCR2());
        int p, user, rw;
        p = frame->err_code & 0x1;
        rw = (frame->err_code & 0x2) >> 1;
        user = (frame->err_code & 0x4) >> 2;
        printf(" %x %x %x", p, rw, user);   
    }
    if(frame->int_no == 13)
    {
        printf(" with error code: %x", frame->err_code);
    }

    asm volatile("cli");
    asm volatile("hlt");
}

void registerExceptionHandlers()
{
    for(int i=0;i<32;i++)
    {
        registerInterruptHandler(i, exceptionHandler);
    }
}