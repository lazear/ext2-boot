/*
vga.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
Implementation of text-mode vga driver for baremetal
*/

#include "defs.h"

char* VGA_MEMORY = 0x000B8000;

int CURRENT_X = 0;
int CURRENT_Y = 0;
int CURRENT_ATTRIB = VGA_COLOR(VGA_WHITE, VGA_BLACK);

int vga_current_x() { return CURRENT_X; }
int vga_current_y() { return CURRENT_Y; }

/* Move the screen's cursor to the specified pos and line 
 * x is the char position, y is the line */ 
void vga_move_cursor( uint16_t x, uint16_t y ) {
	unsigned temp;
	temp = (y * 80) + x;

	outb(0x3D4, 14);
	outb(0x3D5, temp >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, temp);
}

void vga_update_cursor() {
	vga_move_cursor(CURRENT_X/2, CURRENT_Y);
}

void vga_setcolor(int color) {
	CURRENT_ATTRIB = color;
}

void vga_clear() {
	char* vga_address = VGA_MEMORY;

	const long size = 80 * 25;
	for (long i = 0; i < size; i++ ) { 
		*vga_address++ = 0; // character value
		*vga_address++ = CURRENT_ATTRIB; // color value
	}
	CURRENT_Y = 0;
	CURRENT_X = 0;
}

/* Scroll the screen up one line */
void vga_scroll()
{
	if( CURRENT_Y >= 25)
	{
		uint8_t* vga_addr = VGA_MEMORY;
		uint8_t temp = CURRENT_Y - 24;
		memcpy(vga_addr, vga_addr + temp * 160, (25 - temp) * 160 * 2);
		//memset(vga_addr + 24 * 160, 0, 160);
		CURRENT_Y = 24;
	}
	vga_update_cursor();
}

void vga_overwrite_color(int color, int start_x, int start_y, int end_x, int end_y) {
	char *vga_address = VGA_MEMORY;
	int sizeend = 2*end_x + (160* start_y);
	//const uint32_t size = 80*25;

	for (int i = (start_x+1 + (160*start_y)); i < sizeend; i += 2) {
		
		vga_address[i] = color;
	}
}

// kernel level putc, designate x and y position
void vga_kputc(char c, int x, int y) {
	char *vga_address = VGA_MEMORY + (x + y * 160);
	if (isascii(c))
		*vga_address = c | (CURRENT_ATTRIB << 8);
}

void vga_kputs(char* s, int x, int y) {
	int i = 0;
	while (*s != 0) {
		vga_kputc(*s, x+=2, y);
		s++;

	}
}

void vga_puts(char* s) {
	int i = 0;
	while (*s != 0) {
		vga_putc(*s);
		*s++;

	}
}

// Automatically update text position, used in vga_puts
void vga_putc(char c) {
	if (c == '\n') {
		CURRENT_Y += 1;
		CURRENT_X = 0;
		return;
	}
	if (c == '\b') {
		CURRENT_X -= 2;
		vga_kputc(' ', CURRENT_X, CURRENT_Y);
		return;
	}
	if (c == '\t') {
		while(CURRENT_X % 16)
			CURRENT_X++;
		CURRENT_X += 2;

			if (CURRENT_X >= 160) {
		CURRENT_X = 0;
		CURRENT_Y += 1;
	}
		return;
	}
	if (c=='\q') {

	}
	vga_kputc(c, CURRENT_X, CURRENT_Y);
	CURRENT_X += 2;
	if (CURRENT_X >= 160) {
		CURRENT_X = 0;
		CURRENT_Y += 1;
	}
	vga_scroll();

}

void vga_pretty(char* s, int color) {
	int start_x = CURRENT_X;
	int start_y = CURRENT_Y;

	vga_puts(s);

	vga_overwrite_color(color, start_x, start_y, 80, CURRENT_Y);

}

void printx(char* msg, uint32_t i) {
	vga_puts(msg);
	vga_puts(itoa(i, 16));
	vga_putc('\n');
}