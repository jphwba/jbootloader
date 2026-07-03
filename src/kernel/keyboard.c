#include "keyboard.h"
#include "idt.h"
#include "pic.h"
#include "io.h"

#define KBD_DATA_PORT 0x60
#define KBD_BUFFER_SIZE 256

static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile size_t kbd_head = 0;
static volatile size_t kbd_tail = 0;

static const char scancode_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
};

static void kbd_buffer_push(char c) {
    size_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next == kbd_tail) {
        return;
    }
    kbd_buffer[kbd_head] = c;
    kbd_head = next;
}

static void keyboard_handler(registers_t regs) {
    (void)regs;
    uint8_t scancode = inb(KBD_DATA_PORT);

    if (scancode & 0x80) {
        return;
    }
    if (scancode >= sizeof(scancode_ascii)) {
        return;
    }
    char c = scancode_ascii[scancode];
    if (c != 0) {
        kbd_buffer_push(c);
    }
}

void keyboard_init(void) {
    irq_install_handler(1, keyboard_handler);
    pic_clear_mask(1);
}

char keyboard_getchar(void) {
    if (kbd_tail == kbd_head) {
        return 0;
    }
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}