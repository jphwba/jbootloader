#include "printf.h"
#include "vga.h"
#include <stdarg.h>
#include <stdint.h>

static void print_unsigned(unsigned int value, unsigned int base, int uppercase) {
    static const char lower[] = "0123456789abcdef";
    static const char upper[] = "0123456789ABCDEF";
    const char* digits = uppercase ? upper : lower;

    char buf[32];
    int i = 0;
    if (value == 0) {
        buf[i++] = digits[value % base];
        value /= base;
    } else {
        while (value > 0) {
            buf[i++] = digits[value % base];
            value /= base;
        }
    }

    while (i > 0) {
        terminal_putchar(buf[--i]);
    }
}
static void print_signed(int value) {
    if (value < 0) {
        terminal_putchar('-');
        print_unsigned((unsigned int)value, 10, 0);
    } else {
        print_unsigned((unsigned int)value, 10, 0);
    }
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (const char* p = fmt; *p; p++) {
        if (*p != '%') {
            terminal_putchar(*p);
            continue;
        }
        p++;
        switch(*p) {
            case 'd': {
                int v = va_arg(args, int);
                print_signed(v);
                break;
            }
            case 'u': {
                unsigned int v = va_arg(args, unsigned int);
                print_unsigned(v, 10, 0);
                break;
            }
            case 'x': {
                unsigned int v = va_arg(args, unsigned int);
                print_unsigned(v, 16, 0);
                break;
            }
            case 'X': {
                unsigned int v = va_arg(args, unsigned int);
                print_unsigned(v, 16, 1);
                break;
            }
            case 'p': {
                uintptr_t v = va_arg(args, uintptr_t);
                terminal_writestring("0x");
                print_unsigned((unsigned int)v, 16, 0);
                break;
            }
            case 'c': {
                char v = (char)va_arg(args, int);
                terminal_putchar(v);
                break;
            }
            case 's': {
                const char* v = va_arg(args, const char*);
                terminal_writestring(v ? v : "(null)");
                break;
            }
            case '%': {
                terminal_putchar('%');
                break;
            }
            case '\0': {
                va_end(args);
                return;
            }
            default: {
                terminal_putchar('%');
                terminal_putchar(*p);
                break;
            }
        }
    }
    va_end(args);
}
