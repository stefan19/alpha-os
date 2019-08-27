MBALIGN		equ 1 << 0
MEMINFO		equ 1 << 1
GRAPHICINFO	equ 1 << 2
FLAGS		equ MBALIGN | MEMINFO | GRAPHICINFO
MAGIC		equ 0x1BADB002
CHECKSUM	equ -(MAGIC+FLAGS)

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
	
	dd 0
	dd 0
	dd 0
	dd 0
	dd 0

	dd 0	;linear graphics mode
	dd 1024	;width
	dd 720	;height
	dd 24	;depth

section .bss
align 16
stack_bottom:
resb 16384
stack_top:

section .data
align 0x1000
early_pageDirMem: times 1024 dd 0
early_pageDir equ early_pageDirMem - 0xBFF00000

align 0x1000
lower_pageTMem: times 1024 dd 0
lower_pageT equ lower_pageTMem - 0xBFF00000

align 0x1000
higher_pageTMem: times 1024 dd 0
higher_pageT equ higher_pageTMem - 0xBFF00000

section .text
global _start
_start equ start - 0xBFF00000

start:
mov DWORD [early_pageDir], lower_pageT
or DWORD [early_pageDir], 0x7

mov DWORD [early_pageDir+4*0x300], higher_pageT
or DWORD [early_pageDir+4*0x300], 0x7

;Identity map the first 4 MiB to prevent a crash
mov edi, lower_pageT
mov esi, 0
mov ecx, 0x400
.mapLow:
	mov DWORD [edi], esi
	or DWORD [edi], 0x3
	add edi, 4
	add esi, 0x1000
	loop .mapLow

;Map 0x100000 to 0xC0000000
mov edi, higher_pageT
mov esi, 0x100000
mov ecx, 0x400
.mapHigh:
	mov DWORD [edi], esi
	or DWORD [edi], 0x3
	add edi, 4
	add esi, 0x1000
	loop .mapHigh

mov eax, early_pageDir
mov cr3, eax

mov eax, cr0
or eax, 0x80000000
mov cr0, eax

lea ecx, [StartInHigherHalf]
jmp ecx

StartInHigherHalf:
lea edi, [lower_pageTMem + 4*0x100]
mov esi, 0x100000
mov ecx, 0x300
.unmapLow:
	mov DWORD [edi], 0
	invlpg [esi]
	add edi, 4
	add esi, 0x1000
	loop .unmapLow

mov esp, stack_top

extern kmain
cld
push ebx
call kmain				;jump in the kernel proper

.hang:	
	hlt
	jmp .hang