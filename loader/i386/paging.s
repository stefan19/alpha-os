global switchPageDirectory
switchPageDirectory:
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