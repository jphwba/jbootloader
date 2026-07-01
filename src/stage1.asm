[BITS 16]
[ORG 0x7C00]

%include "config.inc"

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov si, msgstg1
    call print_string

    mov bx, STAGE2_OFFSET
    mov es, ax
    mov dh, 0x00
    mov ch, 0x00
    mov cl, STAGE2_START_SECTOR
    mov al, STAGE2_SECTORS
    mov dl, [boot_drive]
    call disk_read
    jc disk_error

    jmp STAGE2_SEGMENT:STAGE2_OFFSET
disk_error:
    mov si, msg_disk_error
    call print_string
    cli
    hlt

disk_read:
    pusha
    mov di, 3
.retry:
    push ax
    push dx
    mov ah, 0x02
    int 0x13
    pop dx
    pop ax
    jnc .done
    dec di
    jz .fail
    xor ah, ah
    int 0x13
    jmp .retry
.fail:
    stc
    popa
    retry
.done:
    clc
    popa
    ret
print_string:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .end
    int 0x10
    jmp .loop
.end:
    popa
    ret
boot_drive: db 0
msgstg1: db "[JBootloader] stage1: loading stage2...", 13, 10, 0
msg_disk_error: db "[JBootloader] stage1: disk read failed", 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55