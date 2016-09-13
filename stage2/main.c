#include "defs.h"
#include "crunch.h"

uint32_t HEAP = 	0x00200000;
uint32_t HEAP_START;

static void putpixel(unsigned char* screen, int x,int y, int color);

void stage2_main(uint32_t* mem_info, vid_info* v) {
	//clear the screen
	HEAP_START = HEAP;
	vga_clear();
	vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
	lsroot();

	mmap* m = (mmap*) *mem_info;
	mmap* max = (mmap*) *++mem_info;
	printx("Mode:", v->mode);
	printx("Mode:", v->info);

	vga_puts(itoa(v->info->framebuffer, 16));

	/* Don't use the heap, because it's going to be wiped */
	gfx_context *c = (gfx_context*) 0x000F0000;
	c->pitch = v->info->pitch;
	c->width = v->info->width;
	c->height = v->info->height;
	c->bpp = v->info->bpp;
	c->framebuffer = v->info->framebuffer;
	memcpy(0x000F1000, m, (uint32_t) max - (uint32_t) m);


	elf_load(m, c); 
	/* We should never reach this point */
	for(;;);
}

static void putpixel(unsigned char* screen, int x,int y, int color) {
	unsigned where = x*3 + y*768*4;
	screen[where] = color & 255;              // BLUE
	screen[where + 1] = (color >> 8) & 255;   // GREEN
	screen[where + 2] = (color >> 16) & 255;  // RED
}
