
#define NULL ((void*) 0)
#define malloc(n)	((void*)( (HEAP += n) - n))
#define size_t		uint32_t
#define assert(n)	

extern char* itoa(uint32_t num, char* buffer, int base);
extern void putx(uint32_t i);
extern size_t strlen(char*);
extern char* strrev(char*);
extern void bmain();


char* vid = (char*) 0xB8000;
uint32_t HEAP = 0x00200000;
