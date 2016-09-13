
#ifndef __defs__
#define __defs__
#include <stdint.h>


#define NULL ((void*) 0)
#define malloc(n)	((void*)((HEAP += n) - n))
#define free(x)
#define size_t		uint32_t
#define assert(e)	((e) ? (void) 0 : vga_pretty(#e, VGA_RED))


extern void lsroot();

extern char* itoa(uint32_t num, int base);
extern void putx(char* msg, uint32_t i);
extern size_t strlen( char* s );
extern char* strncat( char* destination, const char* source, size_t n );
extern char* strcat(char *destination, const char* source);
extern char* strncpy(char *dest, const char *src, uint16_t n);
extern char* strcpy(char *dest, const char *src) ;
extern int strncmp(char* s1, char* s2, size_t n);
extern int strcmp(char *s1, char* s2);
extern char* strchr(const char* s, int c);
extern char* strdup(const char* s) ;
extern char* strrev(char* s);
extern void* memcpy(void *s1, const void *s2, uint32_t n);
extern void* memset(void *s, int c, size_t n) ;
extern void* memsetw(void *s, int c, size_t n);
extern void* memmove(void *s1, const void* s2, size_t n);
extern void* memchr(const void* s, int c, size_t n);
extern void* memrchr(const void* s, int c, size_t n) ;
extern int memcmp(const uint8_t* s1, const uint8_t* s2, size_t n);
extern char* strtok(char* s, const char* delim);
extern inline uint8_t inb(uint16_t port);
extern inline void outb(uint16_t port, uint16_t data);
extern inline void insl(int port, void *addr, int cnt);

extern uint32_t HEAP;
extern uint32_t HEAP_START;

#define VGA_BLACK	0x00
#define VGA_BLUE	0x01
#define VGA_GREEN	0x02
#define VGA_CYAN	0x03
#define VGA_RED		0x04
#define VGA_MAGENTA	0x05
#define VGA_BROWN	0x06
#define VGA_LIGHTGREY	0x07
#define VGA_DARKGREY	0x08
#define VGA_LIGHTBLUE	0x09
#define VGA_LIGHTGREEN	0x0A
#define VGA_LIGHTCYAN	0x0B
#define VGA_LIGHTRED	0x0C
#define VGA_LIGHTMAGENTA	0x0D
#define VGA_LIGHTBROWN		0x0E
#define VGA_WHITE		0x0F

#define VGA_COLOR(f, b)	((b << 4) | (f & 0xF))
#define RGB(r,g,b) (((r&0xFF)<<16) | ((g&0xFF)<<8) | (b & 0xFF))

#define isascii(c)		(c >= 0 && c <= 127)
#define isdigit(c)		(c >= '0' && c <= '9')
#define islower(c)		(c >= 'a' && c <= 'z')
#define isupper(c)		(c >= 'A' && c <= 'Z')
#define tolower(c)		(isdigit(c) ? c : (islower(c) ? c : ((c - 'A') + 'a')))
#define toupper(c)		(isdigit(c) ? c : (isupper(c) ? c : ((c - 'a') + 'A')))


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


#endif