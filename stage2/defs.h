
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


#define isascii(c)		(c >= 0 && c <= 127)
#define isdigit(c)		(c >= '0' && c <= '9')
#define islower(c)		(c >= 'a' && c <= 'z')
#define isupper(c)		(c >= 'A' && c <= 'Z')
#define tolower(c)		(isdigit(c) ? c : (islower(c) ? c : ((c - 'A') + 'a')))
#define toupper(c)		(isdigit(c) ? c : (isupper(c) ? c : ((c - 'a') + 'A')))

#endif