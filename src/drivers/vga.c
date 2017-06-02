#include <drivers/vga.h>
#include <stdint.h>
#include <string.h>

static inline char vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline short vga_entry(unsigned char uc, char color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

int terminal_row;
int terminal_column;
char terminal_color;
short* terminal_buffer;

void vga_init() {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (short*) 0xC00B8000;

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void vga_setcolor(char color) {
    terminal_color = color;
}

void vga_setchar(char c, char color, int x, int y) {
    terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, color);
}

void vga_scroll() {
    if ( terminal_row >= VGA_HEIGHT ) {
        uint32_t temp = terminal_row - VGA_HEIGHT + 1;

        // Move buffer up
        memcpy(terminal_buffer, terminal_buffer + temp * VGA_WIDTH, (VGA_HEIGHT - temp) * VGA_WIDTH * 2);

        // Clear last line
        memsetw(terminal_buffer + (VGA_HEIGHT - temp) * VGA_WIDTH, vga_entry(' ', terminal_color), VGA_WIDTH);

        // Fix pointer
        terminal_row = VGA_HEIGHT - 1;
    }
}

void vga_putchar(char c) {
    if ( c != '\n' ) {
        vga_setchar(c, terminal_color, terminal_column, terminal_row);
    }

    terminal_column++;

    if ( terminal_column == VGA_WIDTH || c == '\n' ) {
        terminal_row++;
        terminal_column = 0;
    }

    vga_scroll();
}

void vga_write(const char* data, int size) {
    for ( int i = 0; i < size; i++ ) {
        vga_putchar(data[i]);
    }
}

void vga_writestring(const char* data) {
    vga_write(data, strlen(data));
}
