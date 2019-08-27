extern syscall_main

global syscall_handler
syscall_handler:
    push ds
    push es
    push fs
    push gs
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp

    push esp
    cld
    call syscall_main
    add esp, 4

    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    pop ds
    pop es
    pop fs
    pop gs
    iret

;syscall_write(const char* str)
global syscall_write
syscall_write:
    mov eax, 2
    mov ebx, [esp + 4]
    int 0x80
    ret

;syscall_exit()
global syscall_exit
syscall_exit:
    mov eax, 0
    int 0x80
    ret