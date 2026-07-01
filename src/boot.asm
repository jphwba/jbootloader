[BITS 16] ; compile in 16-bit real mode
[ORG 0x7c00] ; set origin point to 0x7C00


CODE_OFFSET equ 0x8 ;constants for GDT segment offsets
DATA_OFFSET equ 0x10

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDRESS equ 0x100000

start:
    cli             ; Clear/disable Interupts
    mov ax, 0x00    ; clears out:
    mov ds, ax      ; data
    mov es, ax      ; extra
    mov ss, ax      ; stack
    mov sp, 0x7c00  ; Set stack pointer to 0x7C00
    sti             ; reenable interupts

;Load kernel
mov bx, KERNEL_LOAD_SEG ; Destination segment pointer 0x1000
mov dh, 0x00            ; Head 0/first drive
mov dl, 0x80
mov cl, 0x02            ; Sector 2
mov ch, 0x00            
mov ah, 0x02            ; Read function code 0x02
mov al, 8               ; Read first 8 sectors
int 0x13

jc disk_read_error      ; set carry flag if disk read fails then go to disk_read_error

load_PM:
    cli                             ; Disable interupts/clear
    lgdt[gdt_descriptor]            ; Load global descriptor table (GDT)
    mov eax, cr0                    ; Read control register 0
    or eax, 1                       ; Set protection enable bit to 1
    mov cr0, eax                    ; Write changes
    jmp CODE_OFFSET: PModeMain      ; update code secment register to CODE_OFFSET

disk_read_error:
    hlt ;halt program

; GDT
gdt_start:
    dd 0x0
    dd 0x0

    ; Code segment descriptor
    dw 0xFFFF ;Limit
    dw 0x0000 ;Base
    db 0x00 ;Base
    db 10011010b ; Access byte
    db 11001111b ;Flags
    db 0x00 ;Base

    ; Data segment descriptor
    dw 0xFFFF ;Limit
    dw 0x0000 ;Base
    db 0x00 ;Base
    db 10010010b ; Access byte, changed 1 (code segment) to 0 (data segment) -> Executable bit
    db 11001111b ;Flags
    db 0x00 ;Base

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size fo GDT -1
    dd gdt_start

[BITS 32]
PModeMain:
    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    mov ebp, 0x9C00
    mov esp, ebp

    in al, 0x92
    or al, 2
    out 0x92, al
    jmp CODE_OFFSET:KERNEL_START_ADDRESS

times 510 - ($ - $$) db 0 ; fill spaces with 0 until byte 510

dw 0xAA55 ; bytes 511 and 512 are 55 and AA





; https://wiki.osdev.org/