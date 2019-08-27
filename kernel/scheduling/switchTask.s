section .text
global _switchTask
;void switchTask(task_t* current, task_t* next)
_switchTask:
    push eax
    push ecx
    push edx
    push ebx
    push esi
    push edi
    push ebp

    mov edi, [esp + 32]         ;EDI = Address of the current task structure
    mov [edi], esp              ;Save stack pointer of old task
    
    ;Load next task's state
    mov esi, [esp + 36]          ;ESI = Address of the next task structure

    ;Change TSS kernel stack
    push dword [esi + 4]
    call tssChangeESP0
    add esp, 4

    mov esp, [esi]              ;Stack pointer of next task
    mov eax, [esi + 8]          ;Page directory of next task
    mov ecx, cr3

    cmp eax, ecx
    je .doneVAS
    mov cr3, eax

.doneVAS:
    pop ebp
    pop edi
    pop esi
    pop ebx
    pop edx
    pop ecx
    pop eax

    ret


;void createStack(task_t* new, uint32_t entry, uint32_t stack)
global createStack
createStack:
    push ebp
    mov ebp, esp

    mov eax, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]

    mov esp, edx
    push ecx
    push 0
    push 0
    push 0
    push 0
    push 0
    push 0
    push 0
    mov [eax], esp

    mov esp, ebp
    pop ebp
    ret

global switchToUserMode
extern tssChangeESP0
;switchToUserMode (uint32_t stack, uint32_t entry)
switchToUserMode:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [esp + 4]
    mov ecx, [esp + 8]

    push 0x23
    push eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x1B
    push ecx
    iret

global jmpToKernelThread
;jmpToKernelThread(uint32_t entry)
jmpToKernelThread:
    sti
    jmp [esp + 4]