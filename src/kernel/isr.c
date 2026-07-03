#include "idt.h"
#include "vga.h"

static const char* exception_messages[32] = {
    "Divide-by-zero",
    "Debug",
    "non=maskable interupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceededd",
    "Invalid opcode",
    "Device not avaliable",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "page fault",
    "Reserved",
    "x87 floating point exception",
    "Alignment check",
    "Machine Check",
    "SIMD floating point exception",
    "Virtualisation exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved",
};

void isr_handler(registers_t regs) {
    terminal_set_colour(VGA_COLOUR_WHITE, VGA_COLOUR_RED);
    terminal_writestring("\n[PANIC] ");
    terminal_writestring(exception_messages[regs.int_no]);
    terminal_writestring(" (interrupt ");
    terminal_write_dec(regs.int_no);
    terminal_writestring(", error code ");
    terminal_write_hex(regs.err_code);
    terminal_writestring(")\n eip=");
    terminal_write_hex(regs.eip);
    terminal_writestring(" cs=");
    terminal_write_hex(regs.cs);
    terminal_writestring(" elflags=");
    terminal_write_hex(regs.eflags);
    terminal_writestring("\n");
    terminal_set_colour(VGA_COLOUR_LIGHT_GREY, VGA_COLOUR_BLACK);

    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("htl");
    }
}