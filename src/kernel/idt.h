#indef IDT_H
#define IDT_H
#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags;
} registers_t;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install(void);

typedef void (*isr_t)(registers_t regs);
void irq_install_handler(uint8_t irq, isr_t handler);
void irq_uninstall_handler(uint8_t irq);
#endif