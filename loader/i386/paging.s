global switchPageDirectory
switchPageDirectory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

global enablePaging
enablePaging:
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    ret

global disablePaging
disablePaging:
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
    ret

global invalidatePage
invalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret