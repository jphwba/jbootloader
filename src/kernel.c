#include "kernel.h"
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/pit.h"
#include "kernel/keyboard.h"
#include "kernel/printf.h"
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

void kernel_main(BootInfo* info){
    terminal_init();
    terminal_writestring("JBootloader kernel\n");
    kprintf("boot_drive=0x%x mmap_entries=%u kernel_size=%u bytes\n\n", info->boot_drive, info->mmap_entry_count, info->kernel_size);
    idt_install();
    pic_remap(PIC1_OFFSET, PIC2_OFFSET);
    pit_init(100);
    keyboard_init();
    __asm__ volatile ("sti");
    terminal_writestring("IDT Installed, PIC remapped, PIT and keyboard ready \n");
    terminal_writestring("Type something:\n");

    for (;;) {
        char c = keyboard_getchar();
        if (c != 0) {
            terminal_putchar(c);
        }
        __asm__ volatile ("hlt");
    }
}