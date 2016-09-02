/*
string.c
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

Implementation of string library for baremetal project
*/

#include "defs.h"
#include <stdint.h>

// Returns length of a null-terminated string
size_t strlen( char* s ) {
	char* p = s;
	uint32_t i = 0;
	while (*p++ != 0 ) i++;
	return i;
}


// Appends a copy of the source string to the destination string
char* strncat( char* destination, const char* source, size_t n ) {
	size_t length = strlen(destination);
	int i;
	for (i = 0; i < n && source[i] != '\0'; i++)
		destination[length+i] = source[i];
	destination[length+i] = '\0';
	return destination;
}

// wrapper for strncat
char* strcat(char *destination, const char* source) {
	return strncat(destination, source, strlen(source));
}


// Copy first n characters of src to destination
char* strncpy(char *dest, const char *src, uint16_t n) {
    uint16_t i;

   for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';

   return dest;
}

// Copy all of str to dest
char* strcpy(char *dest, const char *src) {
	return strncpy(dest, src, strlen(src));
}


int strncmp(char* s1, char* s2, size_t n) {
	for (size_t i = 0; i < n && *s1 == *s2; s1++, s2++, i++)
		if (*s1 == '\0')
			return 0;
	return ( *(unsigned char*)s1 - *(unsigned char*)s2 );
}

int strcmp(char *s1, char* s2) {
	return strncmp(s1, s2, strlen(s1));
}


char* strchr(const char* s, int c) {
	while (*s != '\0')
		if (*s++ == c) return (char*) s;
	return NULL;
}

char* strdup(const char* s) {
	return strcpy((char*) malloc(strlen(s)), s);
}

/*
In place string reverse
*/
char* strrev(char* s) {
	int length = strlen(s) - 1;
	for (int i = 0; i <= length/2; i++) {
		char temp = s[i];
		s[i] = s[length-i];
		s[length-i] = temp;
	}
	return s;
}


void* memcpy(void *s1, const void *s2, uint32_t n) {
	uint8_t* src = (uint8_t*) s2;
	uint8_t* dest = (uint8_t*) s1;

	for (int i = 0; i < n; i++)
		dest[i] = src[i];
	return s1; 
}

void* memset(void *s, int c, size_t n) {
	uint8_t* dest = (uint8_t*) s;
	for (size_t i = 0; i < n; i++)
		dest[i] = c;
	return s;
}

void* memsetw(void *s, int c, size_t n) {
	uint16_t* dest = (uint16_t*) s;
	for (size_t i = 0; i < n; i++) *dest++ = c;
}

void* memmove(void *s1, const void* s2, size_t n) {
	char* dest 	= (char*) s1;
	char* src 	= (char*) s2;
	char* temp = (char*) malloc(n);

	for (int i = 0; i < n; i++)
		temp[i] = src[i];
	for (int i = 0; i < n; i++)
		dest[i] = temp[i];

	free(temp);
	return s1;
}

void* memchr(const void* s, int c, size_t n) {
	uint8_t* b = s;
	while (n--)
		if (*b++ == c)
			return b;
	return NULL;
}

void* memrchr(const void* s, int c, size_t n) {
	return strrev(memchr(strrev(s), c, n));
}

int memcmp(const uint8_t* s1, const uint8_t* s2, size_t n) {

	while (*s1 == *s2 && n--) {
		s1++;
		s2++;
	}
	return ( *(uint8_t*)s1 - *(uint8_t*)s2 );
}


char* strtok(char* s, const char* delim) {
	char* b = NULL;
	static char* ptr, d = NULL;


	if (s) 
		ptr = s;

	if (!ptr && !s)
		return NULL;
	for (int i = 0; i < strlen(delim); i++) {
		b = ptr;
		ptr = strchr(ptr, delim[i]);
		if (!ptr) 
			return b;
		*--ptr = '\0';
		ptr++;
		return b;
	}
	return NULL;
}

inline uint8_t inb(uint16_t port) {
  // "=a" (result) means: put AL register in variable result when finished
  // "d" (_port) means: load EDX with _port
	unsigned char result;
	asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

inline void outb(uint16_t port, uint16_t data) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}

inline void insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

char* itoa(uint32_t num, int base) {
	int i = 0;
	//num = abs(num);
	int len = 8;

	char* buffer = malloc(32);
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
