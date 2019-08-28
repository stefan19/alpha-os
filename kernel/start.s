LOADER_MAGIC    equ 0x8DEE5CAF

section .bss
stack_bottom:
resb 0x4000
stack_top:

section .text
global start
start:
;Hang if the kernel was not booted properly
cmp eax, LOADER_MAGIC
jne .hang

mov esp, stack_top

extern kmain
cld
push ebx
push ecx
push edx
call kmain				;jump in the kernel proper

.hang:	
	hlt
	jmp .hang
