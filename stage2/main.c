#include "defs.h"
#include "crunch.h"

uint32_t HEAP = 	0x00200000;
uint32_t HEAP_START;

static void putpixel(unsigned char* screen, int x,int y, int color);
void parse_config(char* config);

void stage2_main(uint32_t* mem_info, vid_info* v) {
	//clear the screen
	HEAP_START = HEAP;
	vga_clear();
	vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
	lsroot();

	mmap* m = (mmap*) *mem_info;
	mmap* max = (mmap*) *++mem_info;
	printx("Video Mode:", v->mode);
	vga_puts("framebuffer@ 0x");

	vga_puts(itoa(v->info->framebuffer, 16));
	vga_putc('\n');
	/* Don't use the heap, because it's going to be wiped */
	gfx_context *c = (gfx_context*) 0x000F0000;
	c->pitch = v->info->pitch;
	c->width = v->info->width;
	c->height = v->info->height;
	c->bpp = v->info->bpp;
	c->framebuffer = v->info->framebuffer;
	memcpy(0x000F1000, m, (uint32_t) max - (uint32_t) m);

	vga_pretty("Memory map:\n", VGA_LIGHTGREEN);
	while (m < max) {

		vga_puts(itoa(m->base, 16));
		vga_putc('-');
		vga_puts(itoa(m->len + m->base, 16));
		vga_putc('\t');
		//vga_puts(itoa(m->type, 10));
		switch((char)m->type) {
			case 1: {
				vga_puts("Usable Ram\t");

				break;
			} case 2: {
				vga_puts("Reserved\t");
				break;
			} default:
				vga_putc('\n');
		}
				vga_puts(itoa(m->len/0x400, 10));
				vga_puts(" KB\n");
		m++;
	}

	int boot_conf_inode = ext2_find_child("boot.conf", 2);

	char* config = ext2_read_file(ext2_inode(1, boot_conf_inode));
	parse_config(config);


	/* We should never reach this point */
	for(;;);
}

void parse_config(char* config) {
	vga_puts(config);
	

}

static void putpixel(unsigned char* screen, int x,int y, int color) {
	unsigned where = x*3 + y*768*4;
	screen[where] = color & 255;              // BLUE
	screen[where + 1] = (color >> 8) & 255;   // GREEN
	screen[where + 2] = (color >> 16) & 255;  // RED
}
