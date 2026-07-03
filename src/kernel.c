#include "kernel.h"
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/pit.h"
#include "kernel/keyboard.h"
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

void kernel_main(BootInfo* info){
    terminal_init();
    terminal_writestring("JBootloader kernel\n");
    terminal_writestring("boot_drive=");
    terminal_write_hex(info->boot_drive);
    terminal_writestring(" mmap_entries=");
    terminal_write_dec(info->mmap_entry_count);
    terminal_writestring(" kernel_size=");
    terminal_write_dec(info->kernel_size);
    terminal_writestring(" bytes\n\n");

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