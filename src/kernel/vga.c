#include "vga.h"
#include "io.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static const size_t TAB_SIZE = 4;
static uint16_t* const VGA_MEMORY =(uint16_t*)0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_colour;
static uint16_t* terminal_buffer;

static inline uint8_t vga_entry_colour(enum vga_colour fg, enum vga_colour bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char c, uint8_t colour) {
    return (uint16_t)c | ((uint16_t)colour << 8);
}

static void terminal_update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_colour = vga_entry_colour(VGA_COLOUR_LIGHT_GREY, VGA_COLOUR_BLACK);
    terminal_buffer = VGA_MEMORY;
    terminal_clear();
    terminal_enable_cursor(14, 15);
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_colour);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    terminal_update_cursor();
}

void terminal_set_cursor(size_t row, size_t col) {
    if (row >= VGA_HEIGHT) row = VGA_HEIGHT - 1;
    if (col >= VGA_WIDTH) col = VGA_WIDTH -1;
    terminal_row = row;
    terminal_column = col;
    terminal_update_cursor();
}

void terminal_get_cursor(size_t* row, size_t* col) {
    *row = terminal_row;
    *col = terminal_column;
}

void terminal_enable_cursor(uint8_t start_scanline, uint8_t end_scanline) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start_scanline);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end_scanline);
}

void terminal_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void terminal_set_colour(uint8_t fg, uint8_t bg) {
    terminal_colour = vga_entry_colour((enum vga_colour)fg, (enum vga_colour)bg);
}

void terminal_get_colour(uint8_t* fg, uint8_t* bg) {
    *fg = terminal_colour & 0x0F;
    *bg = (terminal_colour >> 4) & 0x0F;
}

static void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x=0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y - 1) * VGA_WIDTH +x] = terminal_buffer[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_colour);
    }
    terminal_row = VGA_HEIGHT - 1;
}

static void terminal_newline(void) {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
        terminal_scroll();
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_newline();
        terminal_update_cursor();
        return;
    }
    if (c == '\r') {
        terminal_column = 0;
        terminal_update_cursor();
        return;
    }
    if (c == '\t') {
        size_t next_stop = (terminal_column / TAB_SIZE + 1) * TAB_SIZE;
        while (terminal_column < next_stop && terminal_column < VGA_WIDTH) {
            terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(' ', terminal_colour);
            terminal_column++;
        }
        if (terminal_column == VGA_WIDTH) {
            terminal_newline();
        }
        terminal_update_cursor();
        return;
    }
    if(c == '\b') {
        if(terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(' ', terminal_colour);
        terminal_update_cursor();
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry((unsigned char)c, terminal_colour);
    if (++terminal_column == VGA_WIDTH) {
        terminal_newline();
    }
    terminal_update_cursor();
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    size_t len = 0;
    while (data[len]) len++;
    terminal_write(data, len);
}

void terminal_write_hex(uint32_t value) {
    static const char digits[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; i++) {
        buf[9-i] = digits[value & 0xF];
        value >>= 4;
    }
    buf[10] = 0;
    terminal_writestring(buf);
}

void terminal_write_dec(uint32_t value) {
    char buf[11];
    int i = 10;
    buf[i--] = 0;
    if (value == 0) {
        buf[i--] = '0';
    } else {
        while (value > 0 && i>=0) {
            buf[i--] = '0' + (value % 10);
            value /= 10;
        }
    }
    terminal_writestring(&buf[i+1]);
}