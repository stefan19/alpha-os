MAGIC       equ 0xE85250D6
ARCH        equ 0
HDRLEN      equ multiboot_header_end - multiboot_header
CHECKSUM    equ -(MAGIC + ARCH + HDRLEN)

section .multiboot
align 8
multiboot_header:
    dd MAGIC
    dd ARCH
    dd HDRLEN
    dd CHECKSUM

align 8
info_request:
    dw 1
    dw 0
    dd info_request_end - info_request
    dd 8
info_request_end:

align 8
framebuffer_tag:
    dw 5
    dw 0
    dd framebuffer_tag_end - framebuffer_tag
    dd 800
    dd 600
    dd 32
framebuffer_tag_end:
align 8
    dw 0
    dw 0
    dd 8
multiboot_header_end:

section .bss
align 16
stack_bottom:
resb 0x4000
stack_top:

section .text
global start
extern loader_main
start:
    mov esp, stack_top
    
    cld
    push ebx
    call loader_main
    
    cli
.hang:
    hlt
    jmp .hang
