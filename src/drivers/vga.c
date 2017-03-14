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

void vga_putchar(char c) {
	vga_setchar(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void vga_write(const char* data, int size) {
	for ( int i = 0; i < size; i++ ) {
		vga_putchar(data[i]);
	}
}

void vga_writestring(const char* data) {
	vga_write(data, strlen(data));
}
