LOADER_MAGIC equ 0x8DEE5CAF

global jump_kernel
;void loader_end(uint32_t entryPoint, uint32_t bitmap, uint32_t bitmapSize, uint32_t mbi)
jump_kernel:
    mov eax, LOADER_MAGIC
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    mov edi, [esp + 20]
    
    jmp [esp + 4]