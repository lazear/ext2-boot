

CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm

CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -c 
LDFLAGS	= -Ttext 0x1000 -o bootloader

all: compile clean

compile:
	nasm -f elf bootstrap.asm -o bootstrap.o

	$(CC) -fno-pic $(CCFLAGS) ext2_bootloader.c
	$(LD) -N -e entry -Ttext 0x7C00 -o stage1 bootstrap.o 
	$(LD) -N -e stage2_main -Ttext 0x20000 -o stage2 ext2_bootloader.o

	objcopy -S -O binary -j .text stage1 stage1.bin
	objcopy -S -O binary stage2 stage2.bin

	dd if=stage1.bin of=boot.img conv=notrunc
clean:

	rm *.o
