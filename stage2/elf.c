/*
elf.c
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
*/

#include "defs.h"
#include "ext2.h"
#include "elf.h"

void elf_objdump(void* data) {
	elf32_ehdr *ehdr = (elf32_ehdr*) data;

	/* Make sure the file ain't fucked */
	assert(ehdr->e_ident[0] == ELF_MAGIC);
	assert(ehdr->e_machine 	== EM_386);
	assert(ehdr->e_type		== ET_EXEC);

	printx("ELF ident ",ehdr->e_ident[0]);              
	printx("Entry ",ehdr->e_entry);            

	/* Parse the program headers */
	elf32_phdr* phdr 		= (uint32_t) data + ehdr->e_phoff;
	elf32_phdr* last_phdr 	= (uint32_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum);
	vga_pretty("Offset   \tVirt Addr\tPhys Addr\tFile Sz\tMem sz \tAlign  \n", VGA_LIGHTMAGENTA);
	while(phdr < last_phdr) {
		vga_puts("   ");
		vga_puts(itoa(phdr->p_offset, 16));
		vga_puts("\t");
		vga_puts(itoa(phdr->p_vaddr, 16));
		vga_putc('\t');
		vga_puts(itoa(phdr->p_paddr, 16));
		vga_putc('\t');
		vga_puts(itoa(phdr->p_filesz, 10));
		vga_putc('\t');
		vga_puts(itoa(phdr->p_memsz, 16));
		vga_putc('\t');
	//	vga_puts(itoa(phdr->p_align, 16));
		vga_putc('\n');
	//	printf("LOAD:\toff 0x%x\tvaddr\t0x%x\tpaddr\t0x%x\n\t\tfilesz\t%d\tmemsz\t%d\talign\t%d\t\n",
	//	 	phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, phdr->p_align);
		phdr++;
	} 

	/* Parse the section headers */
	elf32_shdr* shdr 		= (uint32_t) data + ehdr->e_shoff;
	elf32_shdr* sh_str		= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shstrndx);
	elf32_shdr* last_shdr 	= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shnum);

	char* string_table 		= (uint32_t) data + sh_str->sh_offset;

	shdr++;					// Skip null entry
	int q = 0;

	vga_pretty("Sections:\nIdx   Name\tSize\t\tAddress \tOffset\tAlign\n", VGA_LIGHTMAGENTA);
	while (shdr < last_shdr) {	
		vga_puts(itoa(++q, 10));
		vga_puts("  ");
		vga_puts(string_table + shdr->sh_name);
		if (strlen(string_table + shdr->sh_name) < 6)
			vga_puts("\t");
		vga_putc('\t');
		vga_puts(itoa(shdr->sh_size, 16));
		vga_putc('\t');
		vga_puts(itoa(shdr->sh_addr, 16));
		vga_putc('\t');
		vga_puts(itoa(shdr->sh_offset, 10));
		vga_putc('\t');
		vga_puts(itoa(shdr->sh_addralign, 16));
		vga_putc('\n');

		shdr++;
	}

}


void elf_load() {
	inode* ki = ext2_inode(1,12);
	uint32_t* data = ext2_read_file(ki);
	//uint32_t* data = ext2_file_seek(ext2_inode(1,12), 1024, 0);

	elf32_ehdr * ehdr = (elf32_ehdr*) data; 

	assert(ehdr->e_ident[0] == ELF_MAGIC);

	elf_objdump(data);
	printx("data at: ", data);
	printx("heap at: ", malloc(0));
	free(data);

	elf32_phdr* phdr 		= (uint32_t) data + ehdr->e_phoff;
	elf32_phdr* last_phdr 	= (uint32_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum);
	
	uint32_t off = (phdr->p_vaddr - phdr->p_paddr);

	while(phdr < last_phdr) {
		printx("header: ", phdr->p_paddr);
		memcpy(phdr->p_paddr, (uint32_t)data + phdr->p_offset, phdr->p_filesz);
		phdr++;
	} 



	void (*entry)(void);
	entry = (void(*)(void))(ehdr->e_entry - off);

	// CLEAR OUT THE ENTIRE HEAP

	uint32_t END_OF_HEAP = malloc(0);
	memset(HEAP_START, 0, (END_OF_HEAP - HEAP_START));

	

	printx("entry: ", entry);
	asm volatile("cli");

	entry();

}