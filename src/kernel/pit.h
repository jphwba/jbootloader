#ifndef PIT_H
#define PIT_H
#include <stdint.h>

void pit_init(uint32_t frequency_hz);
uint32_t pit_get_ticks(void);
uint32_t pit_get_frequency(void);
uint32_t pit_get_uptime_ms(void);
uint32_t pit_get_uptime_seconds(void);

void pit_sleep_ms(uint32_t ms);
typedef void (*pit_tick_hook_t)(void);
void pit_set_tick_hook(pit_tick_hook_t hook);
#endif