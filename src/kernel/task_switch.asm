[BITS 32]
global task_switch
task_switch:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov eax, [ebp + 8]
    mov [eax], esp
    mov esp, [ebp + 12]
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret