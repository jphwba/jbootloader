#include "kernel.h"
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/pit.h"
#include "kernel/keyboard.h"
#include "kernel/printf.h"
#include "kernel/pmm.h"
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
    pmm_init(info);
    kprintf("PMM: %u KiB total, %u KiB free, &u KiB used\n", (unsigned int)(pmm_get_total_frames() * PMM_PAGE_SIZE / 1024), (unsigned int)(pmm_get_free_frames() * PMM_PAGE_SIZE / 1024), (unsigned int)(pmm_get_used_frames() * PMM_PAGE_SIZE / 1024));
    void* a = pmm_alloc_page();
    void* b = pmm_alloc_page();
    kprintf("test alloc: page1=0x%x page2=0x%x\n", (unsigned int)(uintptr_t)a, (unsigned int)(uintptr_t)b);
    pmm_free_page(a);
    kprintf("freed page1, now %u KiB free\n", (unsigned int)(pmm_get_free_frames() * PMM_PAGE_SIZE / 1024));
    terminal_writestring("Type something:\n");

    for (;;) {
        char c = keyboard_getchar();
        if (c != 0) {
            terminal_putchar(c);
        }
        __asm__ volatile ("hlt");
    }
}