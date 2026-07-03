CODE_OFFSET             equ 0x8         ; GDT code segment selector
DATA_OFFSET             equ 0x10        ; GDT data segment selector
; stage2
STAGE2_SEGMENT          equ 0x0000
STAGE2_OFFSET           equ 0x8000      ; stage2 is laoded to 0x0000:0x8000
STAGE2_SECTORS          equ 8           ; 8 * 512 = 4KiB reserved for stage2
STAGE2_START_SECTOR     equ 2           ; stage1 is sector 1(LBA0) stage2 starts right after
; kernel
KERNEL_LOAD_SEG         equ 0x1000      ; kernel is first loaded to phys 0x10000 (real mode)
KERNEL_SECTORS          equ 64          ; 64 * 512 = 32 KiB for kernel image
KERNEL_START_SECTOR     equ (STAGE2_START_SECTOR - 1) + STAGE2_SECTORS
KERNEL_START_ADDRESS    equ 0x100000    ; final linked address the kernel runs at

; scratch data areas used for stage2
MMAP_BUFFER_ADDR        equ 0x9000      ; BIOS E820 memory map entries
MMAP_ENTRY_SIZE         equ 24          ; bytes per E820 entry we request
MMAP_MAX_ENTRIES        equ 32

BOOTINFO_ADDR           equ 0x9400      ; Bootinfo struct handed to the kernel

STACK_TOP_PM            equ 0x90000     ; protected mode stack clear of the buffers above