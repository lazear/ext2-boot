#include "defs.h"
uint32_t HEAP = 	0x00200000;

void stage2_main() {
	//clear the screen
	vga_clear();
	vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
	lsroot();

	elf_load();

	for(;;);
}

