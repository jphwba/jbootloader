#include "idt.h"
#include "pic.h"

static isr_t irq_routines[16] = { 0 };

void irq_install_handler(uint8_t irq, isr_t handler) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(uint8_t irq) {
    irq_routines[irq] = 0;
}

void irq_handler(registers_t regs) {
    uint8_t irq = (uint8_t)(regs.int_no - 32);

    isr_t handler = irq_routines[irq];
    if(handler) {
        handler(regs)
    }
    pic_send_eoi(irq);
}