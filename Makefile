
COBJS	= 	stage2/main.o \
			stage2/ext2.o \
			stage2/lib.o \
			stage2/vga.o \
			stage2/elf.o

CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm

CCFLAGS	= -w -fno-pic -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -c 
EXT2UTIL= ../ext2util/ext2util

DISK	= boot.img


all: compile clean

%.o : %.c
	$(CC) $(CCFLAGS) $< -o $@

stage2: $(COBJS)

compile: stage2
	$(AS) -f bin bootstrap.asm -o stage1.bin
	$(AS) -f elf test.s -o test.o
	$(LD) -N -e entry -Ttext 0x00100000 -o test test.o
	$(LD) -N -e stage2_main -Ttext 0x50000 -o stage2.bin $(COBJS) --oformat binary
	dd if=stage1.bin of=$(DISK) conv=notrunc

	cp ../baremetal/bin/kernel.bin ./kernel

	$(EXT2UTIL) -x $(DISK) -wf stage2.bin -i 5
	$(EXT2UTIL) -x $(DISK) -wf kernel -i 12

new:
	dd if=/dev/zero of=boot.img bs=1k count=16k
	sudo mke2fs boot.img

clean:

	rm stage2/*.o
