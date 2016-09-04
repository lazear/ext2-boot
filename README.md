# ext2-boot

### x86 bootloader for ext2 filesystems and elf32 kernels
ext2-boot aims to be a configurable bootloader utilizing the reserved bootloader block from the ext2 specification to load and run 32 bit ELF executables. 
Stage2 is essentially a minimal kernel, with only print-to-screen, ext2 file reading, and ELF execution capabilities.

ext2-boot currently supports BIOS memory mapping, and passes along the information.
Goals of this project are to provide easy-to-read, minimal yet functional code that can be used on native ext2 partitions, and be easier to set up than GRUB for people running Windows.

![alt tag](https://raw.githubusercontent.com/lazear/ext2-boot/master/bootloader.png)

#### To-Do:
* Add in configuration file
* Finish video mode selection code
* Add support for multiboot

#### Requirements:
* [ext2util](https://github.com/lazear/ext2util), or some other way to write the second stage loader to inode #5
* elf32 executable

#### Build
Simply run
```sh
$ make
```
And all necessary files will be compiled and moved onto the ext2 image.

If you need a fresh ext2 disk image:
```sh
$ dd if=/dev/zero of=disk.img bs=1k count=16k
$ mke2fs disk.img
```
ext2util can be used to write files (such as a kernel) to the ext2 image.



