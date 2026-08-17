#include "keyboard.h"
#include "idt.h"
#include "pic.h"
#include "io.h"

#define KBD_DATA_PORT 0x60
#define KBD_BUFFER_SIZE 256
#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36
#define SC_CAPSLOCK 0x3A
#define SC_RELEASE_BIT 0x80

static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile size_t kbd_head = 0;
static volatile size_t kbd_tail = 0;

static void kbd_queue_push(char c) {
    size_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next == kbd_tail) {
        return;
    }
    kbd_buffer[kbd_head] = c;
    kbd_head = next;
}

char keyboard_getchar(void) {
    if (kbd_tail == kbd_head) {
        return 0;
    }
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

static const char scancode_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
};

static const char scancode_ascii_shifted[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
};

static int shift_held = 0;
static int caps_on = 0;
static int is_letter_scancode(uint8_t sc) {
    char base = scancode_ascii[sc];
    return base >= 'a' && base <= 'z';
}

static void keyboard_handler(registers_t regs) {
    (void)regs;
    uint8_t scancode = inb(KBD_DATA_PORT);
    int released = scancode & SC_RELEASE_BIT;
    uint8_t code = scancode & ~SC_RELEASE_BIT;

    if (code == SC_CAPSLOCK) {
        if (!released) {
            caps_on = !caps_on;
        }
        return;
    }
    if (released) {
        return;
    }
    if (code >= sizeof(scancode_ascii)) {
        return;
    }
    int use_shifted = shift_held;
    if (is_letter_scancode(code) && caps_on) {
        use_shifted = !use_shifted;
    }
    char c = use_shifted ? scancode_ascii_shifted[code] : scancode_ascii[code];
    if (c != 0) {
        kbd_queue_push(c);
    }
}

void keyboard_init(void) {
    irq_install_handler(1, keyboard_handler);
    pic_clear_mask(1);
}