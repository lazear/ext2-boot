#include "defs.h"
uint32_t HEAP = 	0x00200000;
uint32_t HEAP_START;

typedef struct _boot_mmap
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
} mmap;

typedef struct vbe {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
} vbe; 


typedef struct _vid_info {
	uint32_t mode;
	vbe* info;
} vid_info;


typedef struct _gfx_context {
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint32_t framebuffer;
} gfx_context;

#define RGB(r,g,b) (((r&0xFF)<<16) | ((g&0xFF)<<8) | (b & 0xFF))

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

//	struct vbe* v2 = v->info;
	vga_puts(itoa(v->info->framebuffer, 16));

	gfx_context *c = (gfx_context*) 0x000F0000;
	c->pitch = v->info->pitch;
	c->width = v->info->width;
	c->height = v->info->height;
	c->bpp = v->info->bpp;
	c->framebuffer = v->info->framebuffer;
	memcpy(0x000F1000, m, (uint32_t) max - (uint32_t) m);




	// for (int i =0; i < 1000; i++) {
	// 	putpixel(v->info->framebuffer, i, i%768, RGB(~i,i,i));
	// 	putpixel(v->info->framebuffer, ~i, i%768, RGB(i,~i,i));
	// //	putpixel(v->info->framebuffer, i, 768-i, RGB(i,i,i));
	// }

	// while (m < max) {
	// 	vga_pretty("ENTRY\n", VGA_LIGHTGREEN);
	// 	vga_puts("base low: ");
	// 	vga_puts(itoa(m->base, 16));
	// 	vga_putc('\t');
	// 	// vga_puts("base high: ");
	// 	// vga_puts(itoa(m->base_high, 16));
	// 	// vga_putc('\t');
	// 	vga_puts("len low: ");
	// 	vga_puts(itoa(m->len, 16));
	// 	vga_putc('\t');
	// 	// vga_puts("len high: ");
	// 	// vga_puts(itoa(m->len_high, 16));
	// 	// vga_putc('\n');
	// 	vga_puts("type: ");
	// 	vga_puts(itoa(m->type, 10));
	// 	vga_putc('\n');
	// 	m++;
	// }


	elf_load(mem_info, c); 

	for(;;);
}




/* only valid for 800x600x16M */
static void putpixel(unsigned char* screen, int x,int y, int color) {
    unsigned where = x*3 + y*768*4;
    screen[where] = color & 255;              // BLUE
    screen[where + 1] = (color >> 8) & 255;   // GREEN
    screen[where + 2] = (color >> 16) & 255;  // RED
}
