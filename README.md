# ext2-boot

### x86 bootloader for ext2 filesystems and elf32 kernels

ext2-boot makes use of the reserved ext2 bootloader inode - #5 to store it's second stage elf loader.
#### To-Do:
* Add in configuration file
* Add in video-mode parser, memory mapping, etc

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



