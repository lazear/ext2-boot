/*
ext2_bootloader.c
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

This is intended to be used as a Stage2 bootloader. As such, only Read
functionality.

This loader will be located at inode 5 so that Stage1 bootloader can easily 
find it.
*/

#include "ext2.h"
#include "defs.h"

#include <stdint.h>

void bmain() {
	puts("Stage2 loaded...");
	for(;;);
}


void puts(char* s) {
	while(*s) {
		*vid = *s;
		s++;
		vid += 2;
	}
}

void putx(uint32_t i) {
	char* s = malloc(10);
	itoa(i, s, 16);
	puts(s);
}
char* itoa(uint32_t num, char* buffer, int base) {
	int i = 0;
	//num = abs(num);
	int len = 8;

	if (base == 2)
		len = 32;
	
	if (num == 0 && base == 2) {
		while(i < len)
			buffer[i++] = '0';
		buffer[i] = '\0';
		return buffer;
	}
/*	if (num == 0 && base == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}*/

	// go in reverse order
	while (num != 0 && len--) {
		int remainder = num % base;
		// case for hexadecimal
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}

	while(len-- && base != 10)
		buffer[i++] = '0';

	buffer[i] = '\0';

	return strrev(buffer);
}
// Returns length of a null-terminated string
size_t strlen( char* s ) {
	char* p = s;
	uint32_t i = 0;
	while (*p++ != 0 ) i++;
	return i;
}


char* strrev(char* s) {
	int length = strlen(s) - 1;
	for (int i = 0; i <= length/2; i++) {
		char temp = s[i];
		s[i] = s[length-i];
		s[length-i] = temp;
	}
	return s;
}

static inline uint8_t inb(uint16_t port) {
  // "=a" (result) means: put AL register in variable result when finished
  // "d" (_port) means: load EDX with _port
	unsigned char result;
	asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

static inline void outb(uint16_t port, uint16_t data) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}

static inline void insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

void* memcpy(void *s1, const void *s2, uint32_t n) {
	uint8_t* src = (uint8_t*) s2;
	uint8_t* dest = (uint8_t*) s1;

	for (int i = 0; i < n; i++)
		dest[i] = src[i];
	return s1; 
}

/*
Wait for IDE device to become ready
check =  0, do not check for errors
check != 0, return -1 if error bit set
*/
int ide_wait(int check) {
	char r;

	// Wait while drive is busy. Once just ready is set, exit the loop
	while (((r = (char)inb(IDE_IO | IDE_CMD)) & (IDE_BSY | IDE_RDY)) != IDE_RDY);

	// Check for errors
	if (check && (r & (IDE_DF | IDE_ERR)) != 0)
		return -1;
	return 0;
}

static void* ide_read(void* b, uint32_t block) {

	int sector_per_block = BLOCK_SIZE / SECTOR_SIZE;	// 1
	int sector = block * sector_per_block;

	ide_wait(0);
	outb(IDE_IO | IDE_SECN, sector_per_block);	// # of sectors
	outb(IDE_IO | IDE_LOW, LBA_LOW(sector));
	outb(IDE_IO | IDE_MID, LBA_MID(sector));
	outb(IDE_IO | IDE_HIGH, LBA_HIGH(sector));
	// Slave/Master << 4 and last 4 bits
	outb(IDE_IO | IDE_HEAD, 0xE0 | (1 << 4) | LBA_LAST(sector));	
	outb(IDE_IO | IDE_CMD, IDE_CMD_READ);
	ide_wait(0);
								// Read only
	insl(IDE_IO, b, BLOCK_SIZE/4);	

}


/* Buffer_read and write are used as glue functions for code compatibility 
with hard disk ext2 driver, which has buffer caching functions. Those will
not be included here.  */
void* buffer_read(int block) {
	return ide_read(malloc(BLOCK_SIZE), block);
}

/* 	Read superblock from device dev, and check the magic flag.
	Return NULL if not a valid EXT2 partition */
superblock* ext2_superblock() {
	superblock* sb = buffer_read(EXT2_SUPER);
	if (sb->magic != EXT2_MAGIC)
		return NULL;
	return sb;
}

block_group_descriptor* ext2_blockdesc() {
	return buffer_read(EXT2_SUPER + 1);
}

inode* ext2_inode(int dev, int i) {
	superblock* s = ext2_superblock();
	block_group_descriptor* bgd = ext2_blockdesc();

	assert(s->magic == EXT2_MAGIC);
	assert(bgd);

	int block_group = (i - 1) / s->inodes_per_group; // block group #
	int index 		= (i - 1) % s->inodes_per_group; // index into block group
	int block 		= (index * INODE_SIZE) / BLOCK_SIZE; 
	bgd += block_group;

	// Not using the inode table was the issue...
	uint32_t* data = buffer_read(bgd->inode_table+block);
	inode* in = (inode*)((uint32_t) data + (index % (BLOCK_SIZE/INODE_SIZE))*INODE_SIZE);
	
	return in;
}

void* ext2_open(inode* in) {
	assert(in);
	if(!in)
		return NULL;
	int num_blocks = in->blocks / (BLOCK_SIZE/SECTOR_SIZE);	
	if (!num_blocks){
		return NULL;
	}

	char* buf = malloc(BLOCK_SIZE*num_blocks);

	for (int i = 0; i < num_blocks; i++) {
		char* data = buffer_read(in->block[i]);
		memcpy((uint32_t)buf+(i*BLOCK_SIZE), data, BLOCK_SIZE);
	}
	return buf;
}


void lsroot() {
	inode* i = ext2_inode(1, 2);			// Root directory

	char* buf = malloc(BLOCK_SIZE*i->blocks/2);

	for (int q = 0; q < i->blocks / 2; q++) {
		char* data = buffer_read(i->block[q]);
		memcpy((uint32_t)buf+(q * BLOCK_SIZE), data, BLOCK_SIZE);
	}

	dirent* d = (dirent*) buf;
	
	int sum = 0;
	int calc = 0;
	do {
		
		// Calculate the 4byte aligned size of each entry
		calc = (sizeof(dirent) + d->name_len + 4) & ~0x3;
		sum += d->rec_len;
		puts(d->name);
		if (d->rec_len != calc && sum == 1024) {
			/* if the calculated value doesn't match the given value,
			then we've reached the final entry on the block */
			//sum -= d->rec_len;
			d->rec_len = calc; 		// Resize this entry to it's real size
		//	d = (dirent*)((uint32_t) d + d->rec_len);
		}

		d = (dirent*)((uint32_t) d + d->rec_len);


	} while(sum < 1024);
	return NULL;
}



uint32_t ext2_read_indirect(uint32_t indirect, size_t block_num) {
	uint32_t* data = buffer_read(indirect);
	return *(uint32_t*) ((uint32_t) data + block_num*4);
}
