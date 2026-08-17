[BITS 32]

global _start
extern kernel_main
extern bss_start
extern kernel_end

;Bootlaoder jumps here
_start:
    mov edi, bss_start
    mov ecx, kernel_end
    sub ecx, edi
    xor eax, eax
    cld
    rep stosb
    push ebx
    call kernel_main
    add esp, 4
    cli
.hang:
    hlt
    jmp .hang
    
times 512-($-$$) db 0