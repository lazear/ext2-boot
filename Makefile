
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
run: compile clean emu



%.o : %.c
	$(CC) $(CCFLAGS) $< -o $@

stage2: $(COBJS)

compile: stage2
	$(AS) -f bin bootstrap.asm -o stage1.bin

	$(LD) -N -e stage2_main -Ttext 0x00050000 -o stage2.bin $(COBJS) --oformat binary

	cp boot.img.bak boot.img
	dd if=stage1.bin of=$(DISK) conv=notrunc

	cp ../baremetal/bin/kernel.bin ./kernel

	$(EXT2UTIL) -x $(DISK) -wf stage2.bin -i 5
	$(EXT2UTIL) -x $(DISK) -wf kernel -i 12

new:
	dd if=/dev/zero of=boot.img bs=1k count=16k
	sudo mke2fs boot.img

emu: 
	qemu-system-i386 -hdb boot.img -curses

clean:

	rm stage2/*.o
