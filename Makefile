

CC	    = /home/lazear/opt/cross/bin/i686-elf-gcc
LD		= /home/lazear/opt/cross/bin/i686-elf-ld
AS		= nasm

CCFLAGS	= -w -fno-builtin -nostdlib -ffreestanding -std=gnu99 -m32 -c 
LDFLAGS	= -Ttext 0x1000 -o bootloader

all: compile clean

compile:
	#nasm -f bin bootstrap.asm -o bootstrap
	nasm -f elf bootstrap.asm -o bootstrap.o
	nasm -f bin stage2.asm -o stage2
	dd if=/dev/zero of=boot.img bs=1k count=8		# make new image
	#dd if=bootstrap of=boot.img conv=notrunc
	#dd if=kernel/stage2 of=ext2 seek=1 conv=notrunc
	$(CC) -fno-pic $(CCFLAGS) ext2_bootloader.c
	$(LD) -N -e read_disk -Ttext 0x7C00 -o stage1 bootstrap.o 
	$(LD) -N -e bmain -Ttext 0x1000 -o stage2 ext2_bootloader.o
	objcopy -S -O binary -j .text stage1 stage1.bin
	objcopy -O binary stage2 stage2.bin

	dd if=stage1.bin of=boot.img conv=notrunc
	dd if=stage2.bin of=boot.img seek=1 conv=notrunc
clean:
	#rm *.bin
	rm *.o
	#rm bootstrap
	#rm stage2
#	rm bootloader