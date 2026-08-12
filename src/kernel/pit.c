#include "pit.h"
#include "idt.h"
#include "io.h"
#include "pic.h"
#define PIT_CHANNEL0    0x40
#define PIT_COMMAND     0x43
#define PIT_BASE_FREQ   1193182

static volatile uint32_t ticks = 0;
static uint32_t configured_hz = 100;
static pit_tick_hook_t tick_hook = 0;

static void pit_handler(registers_t regs) {
    (void)regs;
    ticks++;
    if (tick_hook) {
        tick_hook();
    }
}

void pit_init(uint32_t frequency_hz) {
    configured_hz = frequency_hz;
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

uint32_t pit_get_frequency() {
    uint32_t whole_seconds_ms = (ticks / configured_hz) * 1000;
    uint32_t remainder_ms = ((ticks % configured_hz) * 1000) / configured_hz;
    return whole_seconds_ms + remainder_ms;
}

uint32_t pit_get_uptime_seconds(void) {
    return ticks / configured_hz;
}

void pit_sleep_ms(uint32_t ms) {
    uint32_t target = pit_get_uptime_ms() + ms;
    while ((int32_t)(pit_get_uptime_ms() - target) < 0) {
        __asm__ volatile ("hlt");
    }
}

void pit_set_tick_hook(pit_tick_hook_t hook) {
    tick_hook = hook;
}

