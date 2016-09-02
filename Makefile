
COBJS	= 	stage2/main.o \
			stage2/ext2.o \
			stage2/lib.o \
			stage2/vga.o \
			stage2/elf.o

CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm

CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -c 
LDFLAGS	= -Ttext 0x1000 -o bootloader

all: compile clean

%.o : %.c
	$(CC) -fno-pic $(CCFLAGS) $< -o $@

stage2: $(COBJS)

compile: stage2
	nasm -f bin bootstrap.asm -o stage1.bin

	$(LD) -N -e stage2_main -Ttext 0x50000 -o stage2.bin $(COBJS) --oformat binary

	dd if=stage1.bin of=boot.img conv=notrunc
	../ext2util/ext2util -x boot.img -wf stage2.bin -i 5
	../ext2util/ext2util -x boot.img -wf kernel -i 12

clean:

	rm stage2/*.o
