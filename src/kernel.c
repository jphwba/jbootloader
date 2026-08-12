#include "kernel.h"
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/pit.h"
#include "kernel/keyboard.h"
#include "kernel/printf.h"
#include "kernel/pmm.h"
#include "kernel/paging.h"
#include "kernel/heap.h"
#include "kernel/task.h"
#include "kernel/shell.h"

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

static void task_spinner(void) {
    static const char frames[] = {'|', '/', '-', '\\'};
    int i = 0;
    for (;;) {
        terminal_put_at(0, 79, frames[i % 4]);
        i++;
        pit_sleep_ms(250);
    }
}

static void task_counter(void) {
    unsigned int n = 0;
    for (;;) {
        terminal_put_at(0, 78, (char)('0' + (n % 10)));
        n++;
        pit_sleep_ms(500);
    }
}

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
    paging_init();
    terminal_writestring("Paging enabled, first 16Mib mapped)\n");
    heap_init();
    terminal_writestring("kernel heap ready");
    scheduler_init("shell");
    task_create("spinner", task_spinner);
    task_create("counter", task_counter);
    pit_set_tick_hook(scheduler_tick);
    terminal_writestring("Scheduler running started background tasks\n\n");
    shell_run();
    }