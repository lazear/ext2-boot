#include "defs.h"
uint32_t HEAP = 	0x00200000;
uint32_t HEAP_START;

typedef struct _boot_mmap
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
} mmap;

// struct ModeInfoBlock {
//   uint16_t attributes;
//   uint8_t winA,winB;
//   uint16_t granularity;
//   uint16_t winsize;
//   uint16_t segmentA, segmentB;
//   VBE_FAR(realFctPtr);
//   uint16_t pitch; // bytes per scanline
 
//   uint16_t Xres, Yres;
//   uint8_t Wchar, Ychar, planes, bpp, banks;
//   uint8_t memory_model, bank_size, image_pages;
//   uint8_t reserved0;
 
//   uint8_t red_mask, red_position;
//   uint8_t green_mask, green_position;
//   uint8_t blue_mask, blue_position;
//   uint8_t rsv_mask, rsv_position;
//   uint8_t directcolor_attributes;
 
//   uint32_t physbase;  // your LFB (Linear Framebuffer) address ;)
//   uint32_t reserved1;
//   uint16_t reserved2;
// } __attribute__((packed));

struct VbeInfoBlock {
   char VbeSignature[4];             // == "VESA"
   uint16_t VbeVersion;                 // == 0x0300 for VBE 3.0
   uint16_t OemStringPtr[2];            // isa vbeFarPtr
   uint8_t Capabilities[4];
   uint16_t VideoModePtr[2];         // isa vbeFarPtr
   uint16_t TotalMemory;             // as # of 64KB blocks
} __attribute__((packed));


void stage2_main(uint32_t* mem_info, uint32_t* vid_info) {
	//clear the screen
	HEAP_START = HEAP;
	vga_clear();
	vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
	lsroot();

	mmap* m = (mmap*) *mem_info;
	mmap* max = (mmap*) *++mem_info;

	printx("VIDMAP: ", *vid_info);
	printx("VIDMAP: ", *++vid_info);
	printx("VIDMAP: ", *++vid_info);
	printx("MEMMAP: ", mem_info);
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
	vid_info--;
	struct VbeInfoBlock* vib = *--vid_info;
	printx("mode pointers ", vib->VideoModePtr[0]);
	printx("", (uint32_t)&vib->VideoModePtr[0] - (uint32_t)vib);
	uint16_t* ptr = vib->VideoModePtr[0];
	while(*ptr != 0xFFFF)
		printx("ptr: ", *ptr++);



	//elf_load(); 

	for(;;);
}

