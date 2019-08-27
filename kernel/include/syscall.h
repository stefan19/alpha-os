#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef struct
{
    uint32_t ebp, edi, esi, edx, ecx, ebx, eax;
} syscall_regs;

void syscall_write(const char* str);
void syscall_exit();

#endif