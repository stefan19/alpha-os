global outb
outb:
    mov al, byte [esp+8]
    mov dx, [esp+4]
    out dx, al
    ret

global outw
outw:
    mov ax, word [esp+8]
    mov dx, [esp+4]
    out dx, ax
    ret

global outd
outd:
    mov eax, dword [esp+8]
    mov dx, [esp+4]
    out dx, eax
    ret

global inb
inb:
    mov dx, [esp+4]
    in al, dx
    ret

global inw
inw:
    mov dx, [esp+4]
    in ax, dx
    ret

global ind
ind:
    mov dx, [esp+4]
    in eax, dx
    ret

global readCR2
readCR2:
    mov eax, cr2
    ret

global readCR3
readCR3:
    mov eax, cr3
    ret

global readFlags
readFlags:
    pushf
    pop eax
    ret

global restoreFlags
restoreFlags:
    push DWORD [esp + 4]
    popf
    ret