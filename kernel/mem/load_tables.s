section .bss
gdtr    resw 1
        resd 1

idtr    resw 1
        resd 1

section .text
global gdtLoad
gdtLoad:
    mov eax, [esp + 4]
    mov [gdtr + 2], eax
    mov ax, [esp + 8]
    mov [gdtr], ax
    lgdt [gdtr]
    jmp 0x08: .continue
.continue:
    mov ax, 0x10
    mov ds, ax
    mov fs, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

global tssLoad
tssLoad:
    mov ax, 0x28
    ltr ax
    ret

global idtLoad
idtLoad:
    mov eax, [esp + 4]
    mov [idtr + 2], eax
    mov ax, [esp + 8]
    mov [idtr], ax
    lidt [idtr]
    ret

global loadPageDirectory
loadPageDirectory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

global enablePaging
enablePaging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

global invalidatePage
invalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret