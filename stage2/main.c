#include "defs.h"


void stage2_main() {
	//clear the screen
	vga_clear();
	vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
	lsroot();

	elf_load();

	for(;;);
}

