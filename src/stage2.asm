; loaded via stage1 at 0x0000:0x8000

[BITS 16]

%include "src/config.inc"

[ORG STAGE2_OFFSET]

stage2_start:
    mov [boot_drive], dl        ; DL still holds boot drive

    mov si, msgstg2
    call print_string
    call check_lba_ext
    jc .no_ext

    mov si, msg_loadingkern
    call print_string


    ; load kernel
    mov eax, KERNEL_START_SECTOR
    mov cx, KERNEL_SECTORS
    mov bx, KERNEL_LOAD_SEG
    mov es, bx
    xor bx, bx
    mov dl, [boot_drive]
    call disk_read_lba
    jc disk_error

    mov si, msg_a20
    call print_string
    call enable_a20

    mov si, msg_mmap
    call print_string
    call detect_memory

    mov si, msg_pmode
    call print_string

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_OFFSET:PModeMain

.no_ext:
    mov si, msg_no_ext
    call print_string
    jmp halt
disk_error:
    mov si, msg_disk_error
    call print_string
halt:
    cli
    hlt

; verify BIOS INT 13h extensions are on the bootdrive

check_lba_ext:
    pusha
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc .unsupported
    cmp bx, 0xAA55
    jne .unsupported
    popa
    clc
    ret
.unsupported:
    popa
    stc
    ret

disk_read_lba:
    pusha
    mov [dap_lba], eax
    mov [dap_count], cx
    mov [dap_offset], bx
    mov [dap_segment], es
    mov si, 3

.attempt:
    mov ah, 0x00
    int 0x13

    mov ah, 0x42
    mov dl, [boot_drive]
    push si
    mov si, dap
    int 0x13
    pop si
    jnc .success

    dec si
    jnz .attempt

    popa
    stc
    ret
.success:
    popa
    clc
    ret

enable_a20:
    pusha
    mov ax, 0x2401
    int 0x15            ; enable A20 function

    in al, 0x92
    and al, 0xFE        ; keep bit 0 clear
    or al, 0x02         ; set bit 1 to enable A20 gate
    out 0x92, al
    popa
    ret

detect_memory:
    pusha
    xor ebx, ebx
    xor bp, bp                  ; entries found so far
    mov di, MMAP_BUFFER_ADDR
.loop:
    mov eax, 0xE820
    mov ecx, MMAP_ENTRY_SIZE
    mov edx, 0x534D4150         ; SMAP
    mov dword [es:di + 20], 1   ; default ACPI extended attribute bit
    int 0x15
    jc .done                    ; error/end of list
    cmp eax, 0x534D4150
    jne .done                   ; BIOS doesnt support call
    cmp bp, MMAP_MAX_ENTRIES
    jae .done
    inc bp
    add di, MMAP_ENTRY_SIZE
    test ebx, ebx
    jz .done                    ; EBX=0 means it is last entry
    jmp .loop
.done:
    mov [mmap_count], bp
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
mmap_count: dw 0

dap:
    db 0x10         ; packet size
    db 0            ; reserved
dap_count:
    dw 0            ; sector count
dap_offset:
    dw 0            ; destination offset
dap_segment:
    dw 0            ; destination segment
dap_lba:
    dq 0            ; starting LBA

msgstg2:            db "[JBootloader] stage2: initialising...", 13, 10, 0
msg_loadingkern:    db "[JBootloader] loading kernel...", 13, 10, 0
msg_a20:            db "[JBootloader] enabling A20...", 13, 10, 0
msg_mmap:           db "[JBootloader] reading memory map...", 13, 10, 0
msg_pmode:          db "[JBootloader] switching to protected mode...", 13, 10, 0
msg_disk_error:     db "[JBootloader] disk read failed", 13, 10, 0
msg_no_ext:         db "[JBootloader] BIOS LBA extensions not supported", 13, 10, 0

; GDT
gdt_start:
    dd 0x0
    dd 0x0
    ; code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
    ; data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1
    dd gdt_start

[BITS 32]
PModeMain:
    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, STACK_TOP_PM

    mov esi, KERNEL_LOAD_SEG << 4
    mov edi, KERNEL_START_ADDRESS
    mov ecx, (KERNEL_SECTORS * 512)/4
    cld
    rep movsd

    mov edi, BOOTINFO_ADDR
    xor eax, eax
    mov al, [boot_drive]
    mov [edi], eax                              ; boot_drive
    xor eax, eax
    mov ax, [mmap_count]
    mov [edi + 4], eax                          ; mmap entry count
    mov dword [edi + 8], MMAP_BUFFER_ADDR       ; mmap addr
    mov dword [edi + 12], KERNEL_SECTORS * 512  ; kernel_size

    mov ebx, BOOTINFO_ADDR
    jmp CODE_OFFSET:KERNEL_START_ADDRESS
times (STAGE2_SECTORS * 512) - ($ - $$) db 0