#include "pit.h"
#include "idt.h"
#include "io.h"
#include "pic.h"
#define PIT_CHANNEL0    0x40
#define PIT_COMMAND     0x43
#define PIT_BASE_FREQ   1193182

static volatile uint32_t ticks = 0;

static void pit_handler(registers_t regs) {
    (void)regs;
    ticks++;
}

void pit_init(uint32_t frequency_hz) {
    uint32_t divisor = PIT_BASE_FREQ / frequency_hz;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    irq_install_handler(0, pit_handler);
    pic_clear_mask(0);
}

uint32_t pit_get_ticks(void) {
    return ticks;
}