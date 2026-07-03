[BITS 32]

global _start
extern kernel_main

;Bootlaoder jumps here
_start:
    push ebx
    call kernel_main
    add esp, 4
    cli
.hang:
    hlt
    jmp .hang
    
times 512-($-$$) db 0