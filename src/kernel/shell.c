#include "shell.h"
#include "vga.h"
#include "printf.h"
#include "keyboard.h"
#include "pmm.h"
#include "pit.h"
#include "fat16.h"
#include "task.h"
#include "io.h"
#include <stddef.h>
#define LINE_MAX 128
#define CAT_BUF_SIZE 4096

static int fs_ready = 0;
static int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static uint32_t shell_read_line(char* buf, uint32_t max) {
    uint32_t len = 0;
    for (;;) {
        char c = keyboard_getchar();
        if (c == 0) {
            __asm__ volatile ("hlt");
            continue;
        }
        if (c == '\n') {
            terminal_putchar('\n');
            buf[len] = 0;
            return len;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                terminal_putchar('\b');
            }
            continue;
        }
        if (c == '\t') {
            continue;
        }
        if (len + 1 < max) {
            buf[len++] = c;
            terminal_putchar(c);
        }
    }
}

static void cmd_help(void) {
    terminal_writestring(
        "Commands:\n"
        "help       show this list\n"
        "clear      clear the screen\n"
        "mem        show PPM memory usage\n"
        "uptime     show time since boot\n"
        "echo       print text back\n"
        "ps         list scheduler tasks\n"
        "ls         list files on the fat16 volume\n"
        "cat        print file contents\n"
        "version    show kernel verison (too much work to update version number, stays at 1)\n"
        "panic      trigger divide by zero to test fault handler\n"
        "reboot     reset machine\n"
        "shutdown   power off (only qemu)\n"
    );
}

static void cmd_mem(void) {
    kprintf("PMM: %u KiB total, %u KiB free, %u KiB used\n",
        (unsigned int)(pmm_get_total_frames() * PMM_PAGE_SIZE / 1024),
        (unsigned int)(pmm_get_free_frames() * PMM_PAGE_SIZE / 1024),
        (unsigned int)(pmm_get_used_frames() * PMM_PAGE_SIZE / 1024));
}

static void cmd_uptime(void) {
    uint32_t ms = pit_get_uptime_ms();
    kprintf("up %u.%us (%u ticks at %u Hz)\n",
    (unsigned int)(ms / 1000), (unsigned int)(ms % 1000),
    (unsigned int)pit_get_ticks(), (unsigned int)pit_get_frequency());
}

static void cmd_echo(const char* arg) {
    terminal_writestring(arg);
    terminal_writestring("\n");
}

static void cmd_ps(void) {
    kprintf("%u tasks, currently running: %s\n",
    (unsigned int)scheduler_task_count(), scheduler_current_name());
}

static void ls_cb(const char* name, uint32_t size) {
    kprintf("   %s\t%u bytes\n", name, size);
}

static void cmd_ls(void) {
    if (!fs_ready) {
        terminal_writestring("no filesystem mounted\n");
        return;
    }
    fat16_list(ls_cb);
}

static void cmd_cat(const char* arg) {
    if (!fs_ready) {
        terminal_writestring("no filesystem mounted\n");
        return;
    }
    if (arg[0] == 0) {
        terminal_writestring("usage: cat <file>\n");
        return;
    }
    static uint8_t buf[CAT_BUF_SIZE];
    uint32_t n = fat16_read_file(arg, buf, CAT_BUF_SIZE - 1);
    if (n == 0) {
        terminal_writestring("file not found/empty\n");
        return;
    }
    buf[n] = 0;
    terminal_writestring((const char*)buf);
    terminal_writestring("\n");
}

static void cmd_version(void) {
    terminal_writestring("JBootloader kernel, version idk\n");
}

static void cmd_panic(void) {
    terminal_writestring("You have 100 cookies but 0 friends");
    volatile int numerator = 100;
    volatile int zero =0;
    volatile int x = numerator / zero;
    (void)x;
}

static void cmd_reboot(void) {
    terminal_writestring("rebooting...\n");
    uint8_t status;
    do {
        status = inb(0x64);
    } while ( status & 0x02);
    outb(0x64, 0xFE);
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void cmd_shutdown(void) {
    terminal_writestring("shutting down..\n");
    outw(0x604, 0x2000); // check if this works its the old piix4 acpi shutdown port
    // wont work on real hardware
    terminal_writestring("(shutdown didnt work cuh, halting\n)");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void dispatch(char* line) {
    char* arg = line;
    while (*arg && *arg != ' ') arg++;
    if (*arg == ' ') {
        *arg = 0;
        arg++;
        while (*arg == ' ') arg++;
    }

    if (line[0] == 0) return;
    if (streq(line, "help")) cmd_help();
    else if (streq(line, "clear")) terminal_clear();
    else if (streq(line, "mem")) cmd_mem();
    else if (streq(line, "uptime")) cmd_uptime();
    else if (streq(line, "echo")) cmd_echo(arg);
    else if (streq(line, "ps")) cmd_ps();
    else if (streq(line, "ls")) cmd_ls();
    else if (streq(line, "cat")) cmd_cat(arg);
    else if (streq(line, "version")) cmd_version();
    else if (streq(line, "panic")) cmd_panic();
    else if (streq(line, "reboot")) cmd_reboot();
    else if(streq(line, "shutdown")) cmd_shutdown();
    else {
        kprintf("unknown command: %s use 'help'\n", line);
    }
}

void shell_run(void) {
    fs_ready = (fat16_init() == 0);
    if (fs_ready) {
        terminal_writestring("FAT16 volume mounted\n");
    } else {
        terminal_writestring("no fat16 volume found cant use ls/cat");
    }
    terminal_writestring("Type 'help' for commands \n");
    char line[LINE_MAX];
    for (;;) {
        terminal_writestring("> ");
        shell_read_line(line, LINE_MAX);
        dispatch(line);
    }
}
