/*
ext2.h
================================================================================
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
================================================================================
*/


/*

Block groups are found at the address (group number - 1) * blocks_per_group.
Each block group has a backup superblock as it's first block
*/

#include <stdint.h>

#ifndef __baremetal_ext2__
#define __baremetal_ext2__

#define BLOCK_SIZE		1024
#define SECTOR_SIZE		512
#define EXT2_BOOT		0			// Block 0 is bootblock
#define EXT2_SUPER		1			// Block 1 is superblock
#define EXT2_MAGIC		0x0000EF53

typedef struct superblock_s {
	uint32_t inodes_count;			// Total # of inodes
	uint32_t blocks_count;			// Total # of blocks
	uint32_t r_blocks_count;		// # of reserved blocks for superuser
	uint32_t free_blocks_count;	
	uint32_t free_inodes_count;
	uint32_t first_data_block;
	uint32_t log_block_size;		// 1024 << Log2 block size  = block size
	uint32_t log_frag_size;
	uint32_t blocks_per_group;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t mtime;					// Last mount time, in POSIX time
	uint32_t wtime;					// Last write time, in POSIX time
	uint16_t mnt_count;				// # of mounts since last check
	uint16_t max_mnt_count;			// # of mounts before fsck must be done
	uint16_t magic;					// 0xEF53
	uint16_t state;
	uint16_t errors;
	uint16_t minor_rev_level;
	uint32_t lastcheck;
	uint32_t checkinterval;
	uint32_t creator_os;
	uint32_t rev_level;
	uint16_t def_resuid;
	uint16_t def_resgid;
} __attribute__((packed)) superblock;

/*
Inode bitmap size = (inodes_per_group / 8) / BLOCK_SIZE
block_group = (block_number - 1)/ (blocks_per_group) + 1
*/
typedef struct block_group_descriptor_s {
	uint32_t block_bitmap;
	uint32_t inode_bitmap;
	uint32_t inode_table;
	uint16_t free_blocks_count;
	uint16_t free_inodes_count;
	uint16_t used_dirs_count;
	uint16_t pad[7];
} block_group_descriptor;


/*
maximum value of inode.block[index] is inode.blocks / (2 << log_block_size)

Locating an inode:
block group = (inode - 1) / s_inodes_per_group

inside block:
local inode index = (inode - 1) % s_inodes_per_group

containing block = (index * INODE_SIZE) / BLOCK_SIZE
*/
typedef struct inode_s {
	uint16_t mode;			// Format of the file, and access rights
	uint16_t uid;			// User id associated with file
	uint32_t size;			// Size of file in bytes
	uint32_t atime;			// Last access time, POSIX
	uint32_t ctime;			// Creation time
	uint32_t mtime;			// Last modified time
	uint32_t dtime;			// Deletion time
	uint16_t gid;			// POSIX group access
	uint16_t links_count;	// How many links
	uint32_t blocks;		// # of 512-bytes blocks reserved to contain the data
	uint32_t flags;			// EXT2 behavior
	uint32_t osdl;			// OS dependent value
	uint32_t block[15];		// Block pointers. Last 3 are indirect
	uint32_t generation;	// File version
	uint32_t file_acl;		// Block # containing extended attributes
	uint32_t dir_acl;
	uint32_t faddr;			// Location of file fragment
	uint32_t osd2[3];
} inode;

#define INODE_SIZE (sizeof(inode))


/*
Directories must be 4byte aligned, and cannot extend between multiple
blocks on the disk */
typedef struct dirent_s {
	uint32_t inode;			// Inode
	uint16_t rec_len;		// Total size of entry, including all fields
	uint8_t name_len;		// Name length, least significant 8 bits
	uint8_t file_type;		// Type indicator
	uint8_t name[];
} __attribute__((packed)) dirent;

/* IMPORTANT: Inode addresses start at 1 */

typedef struct ide_buffer {
	uint32_t block;				// block number
	uint8_t data[BLOCK_SIZE];	// 1 disk sector of data
} buffer;



#define B_BUSY	0x1		// buffer is locked by a process
#define B_VALID	0x2		// buffer has been read from disk
#define B_DIRTY	0x4		// buffer has been written to


/* Define IDE status bits */
#define IDE_BSY 		(1<<7)	// Drive is preparing to send/receive data
#define IDE_RDY 		(1<<6)	// Clear when drive is spun down, or after error
#define IDE_DF			(1<<5)	// Drive Fault error
#define IDE_ERR			(1<<0)	// Error has occured

#define IDE_IO			0x1F0	// Main IO port
#define IDE_DATA		0x0 	// R/W PIO data bytes
#define IDE_FEAT		0x1 	// ATAPI devices
#define IDE_SECN		0x2 	// # of sectors to R/W
#define IDE_LOW			0x3 	// CHS/LBA28/LBA48 specific
#define IDE_MID 		0x4
#define IDE_HIGH		0x5
#define IDE_HEAD		0x6 	// Select drive/heaad
#define IDE_CMD 		0x7 	// Command/status port
#define IDE_ALT			0x3F6	// alternate status
#define LBA_LOW(c)		((uint8_t) (c & 0xFF))
#define LBA_MID(c)		((uint8_t) (c >> 8) & 0xFF)
#define LBA_HIGH(c)		((uint8_t) (c >> 16) & 0xFF)
#define LBA_LAST(c)		((uint8_t) (c >> 24) & 0xF)

#define IDE_CMD_READ  (BLOCK_SIZE/SECTOR_SIZE == 1) ? 0x20 : 0xC4
#define IDE_CMD_WRITE (BLOCK_SIZE/SECTOR_SIZE == 1) ? 0x30 : 0xC5
#define IDE_CMD_READ_MUL  0xC4
#define IDE_CMD_WRITE_MUL 0xC5



#endif