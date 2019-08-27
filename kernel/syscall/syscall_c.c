#include "syscall.h"
#include "console.h"

void syscall_main(syscall_regs* regs)
{
    switch (regs->eax)
    {
    //Exit syscall
    case 0:
        asm volatile("cli");
        asm volatile("hlt");
        break;
    //Read syscall 
    case 1:
        break;
    //Write syscall
    case 2:
        consoleWriteStr((char*)regs->ebx);
        break;
    default:
        break;
    }
}