BITS 32

section .text
global _start
_start:
	mov eax, 2
	mov ebx, msg
	int 0x80

	mov eax, 0
	int 0x80

section .data
msg db "Hello, exe world!",0
