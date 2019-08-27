#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct interrupt_frame
{
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags;
} interrupt_frame_t;

typedef void (*isr_t)(const interrupt_frame_t*);

void registerInterruptHandler(int no, isr_t handler);

#endif