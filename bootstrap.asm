;								BOOTSTRAP.S
;==============================================================================
;MIT License
;Copyright (c) 2007-2016 Michael Lazear
;
;Permission is hereby granted, free of charge, to any person obtaining a copy
;of this software and associated documentation files (the "Software"), to deal
;in the Software without restriction, including without limitation the rights
;to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;copies of the Software, and to permit persons to whom the Software is
;furnished to do so, subject to the following conditions:
;
;The above copyright notice and this permission notice shall be included in all
;copies or substantial portions of the Software.
;
;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;SOFTWARE.
;==============================================================================
;	MEMORY MAP
; 0x0400 : 0x0500 - memory map info
; 0x2000 : 0x2??? - VESA controller info
; 0x3000 : 
; 0x5000 : 0x7200 Stage2 bootloader
; 0x7C00 : 0x8000 Stage1/1.5 bootloader
; 





[BITS 16]
[ORG 0x7C00]

GLOBAL entry

; Stage1 is responsible for loading Stage1.5, and mapping video modes


entry:

	and dl, 0x80
	jz error

	mov [drive], dl				; BIOS stores drive # in dl

	mov bx, stage_oneandhalf 	; Load into address of Stage1.5
	mov [dest], bx
	call read_disk

	jmp stage_oneandhalf


read_disk:
	mov esi, PACKET		; address of "disk address packet"
	mov ah, 0x42		; extended read
	mov dl, [drive]		; drive number 0 (OR the drive # with 0x80)
	int 0x13
	jc error
	ret

print:
	lodsb
	or al, al		; test for NULL termination
	jz .printdone 
	mov ah, 0eh 
	int 10h
	jmp print
.printdone:
	ret


error:
	mov esi, ext2_error
	call print
	jmp $



; 0x56455341 = VESA
video_map:
	xor eax, eax
	mov es, ax	
	mov bx, 0x2000
	mov di, bx
	mov ax, 0x4F00
	int 0x10

	cmp ax, 0x004F
	jne .error

	;mov [vid_info+4], bx		; Store pointer to video controller array
	mov si, [bx + 0xE]		; Offset to mode pointer
	mov ax, [bx + 0x10]		; Segment to mode pointer
	mov es, ax

	mov di, 0x3000

.loop:
	mov bx, [es:si]			; Load BX with video mode
	cmp bx, 0xFFFF
	je .done				; End of list

	add si, 2
	mov [.mode], bx

	mov ax, 0x4F01			; Get mode info pitch+16, width+18, height+20
	mov cx, [.mode]

	int 0x10
	cmp ax, 0x004F
	jne .error
	xor ax, ax

	mov ax, [es:di + 42]
	mov [.framebuffer], ax

	mov ax, [es:di + 16]	; pitch
	mov bx, [es:di + 18]	; width
	mov cx, [es:di + 20]	; height
	mov dx, [es:di + 25]	; bpp
	mov [.bpp], dx

	add di, 0x100

	cmp ax, [.pitch]
	jne .loop

	cmp bx, [.width]
	jne .loop

	cmp cx, [.height]
	jne .loop

	lea ax, [es:di - 0x100]
	mov [vid_info.array], ax	; Pointer to mode array

.setmode:
	xor ax, ax
	mov ax, 0x4F02		; Function AX=4F02h;
	mov bx, [.mode]
	mov [vid_info], bx
	or bx, 0x4000		; enable LFB

	; int 0x10
	; cmp ax, 0x004F
	; jne .error

.done:
	ret

.error:
	mov si, video_error	
	call print
	jmp $


.mode 			dw 0
.width 			dw 1024
.height 		dw 768
.pitch 			dw 3072
.bpp			dw 0
.framebuffer	dd 0

;==============================================================================
; LBA DATA PACKET
PACKET:
			db	0x10	; packet size (16 bytes)
			db	0		; always 0
count:		dw	4		; number of sectors to transfer
dest:		dw	0		; destination offset (0:7c00)
			dw	0		; destination segment
lba:		dd	1		; put the lba # to read in this spot
			dd	0		; more storage bytes only for big lba's ( > 4 bytes )
;==============================================================================


ext2_success		db 13, "EXT2 Magic Header Good!", 10, 0
ext2_error			db 13, "EXT2 superblock not found", 10, 0
stageonepointfive 	db 13, "Stage1.5 loaded!", 10, 0
video_error			db 13, "VESA ERROR", 10, 0
drive				db 0

vid_info:
.mode	dd 0
.array 	dd 0


times 510-($-$$) db 0           ; Fill up the file with zeros
dw 0AA55h                       ; Last 2 bytes = Boot sector identifyer


;==============================================================================
; END 	LBA SECTOR 0. 
;
; We are now out of the zone loaded by the BIOS
; However, Stage1 contains some useful functions that we can continue 
; to use, since Stage1.5 is directly loaded at the end of stage1 (0x7E00)
;
; Stage1.5 mainly focuses on parsing ext2 data to find the blocks used for
; inode #5, which is reserved by ext2 specific for bootloaders - and we have
; placed the Stage2 loaded there. Memory mapping function is also placed here
;
; BEGIN 	LBA SECTOR 1
;==============================================================================


stage_oneandhalf:
	
	mov esi, stageonepointfive
	call print

	xor bx, bx

	mov ax, [superblock  +56]		; EXT2_MAGUC
	cmp ax, 0xEF53                                          
	jne error 						; Not a valid EXT2 disk

	mov si, ext2_success
	call print

; We need to read the inode table entry for inode 5

	mov ax, [superblock  + 1024 + 8]	; Block_Group_Descriptor->inode_table
	mov cx, 2							
	mul cx								; ax = cx * ax

	mov [lba], ax						; which sector do we read?

	mov ax, 2							; read 1024 bytes
	mov [count], ax	

	mov bx, 0x1000						; copy data to 0x1000
	mov [dest], bx

	call read_disk

	xor bx, bx
	mov bx, 0x1200		; Inode 5 is 0x200 into the first block (index 4 * 128 bytes per inode)
	mov cx, [bx + 28]	; # of sectors for inode
	lea di, [bx + 40]	; address of first block pointer

	mov bx, 0x5000
	mov [dest+2], bx
	mov bx, 0		; where inode5 will be loaded. 0xF0000
	mov [dest], bx
	call read_stagetwo

	call video_map
; Prepare to call the memory mapping function
	xor eax, eax		; Clear EAX
	mov es, eax			; Clear ES
	mov edi, 0x400		; DI = 0x400. ES:DI => 0x00000400
	push edi			; Push start of memory map

	call memory_map
	push edi			; Push end of memory map
	jmp enter_pm 		; Enter protected mode


; Bios function INT 15h, AX=E820h
; EBX must contain 0 on first call, and remain unchanged
; 	on subsequent calls until it is zero again
; Code adapted from OSDEV wiki
; Memory map is 24 byte struct placed at [ES:DI]
memory_map:
	xor ebx, ebx				; ebx must be 0 to start
	mov edx, 0x0534D4150		; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes
	int 0x15
	jc short .fail				; carry set on first call means "unsupported function"

	mov edx, 0x0534D4150		; 
	cmp eax, edx				; on success, eax must have been set to "SMAP"
	jne short .fail

	test ebx, ebx				; ebx = 0 implies list is only 1 entry long (worthless)
	je short .fail

	jmp short .loop

.e820lp:
	mov eax, 0xe820				; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes again
	int 0x15
	jc short .done				; carry set means "end of list already reached"
	mov edx, 0x0534D4150		; repair potentially trashed register
.loop:
	jcxz .skip					; skip any 0 length entries
	cmp cl, 20					; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skip
.notext:
	mov ecx, [es:di + 8]		; get lower uint32_t of memory region length
	or ecx, [es:di + 12]		; "or" it with upper uint32_t to test for zero
	jz .skip					; if length uint64_t is 0, skip entry
	add di, 24
	
.skip:
	test ebx, ebx				; if ebx resets to 0, list is complete
	jne short .e820lp
.done:
	clc							; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.fail:
	stc							; "function unsupported" error exit
	ret

; We enter the loop with:
;	bx = inode pointer
; 	cx = # of sectors to read (2 per block)
;	di = address of first block pointer
; No support for indirect pointers. So keep 
; stage2
read_stagetwo:
	xor ax, ax		; clear ax
	mov dx, ax		; clear dx
	mov ax, [di]	; set ax = block pointer
	mov dx, 2		; multiply by 2 for sectors
	mul dx			; ax = dx * ax

	mov [lba], ax
	mov [dest], bx

	call read_disk

	add bx, 0x400	; increase by 1 kb
	add di, 0x4		; move to next block pointer
	sub cx, 0x2		; reading two blocks
	jnz read_stagetwo
	ret

	nop
	hlt


enter_pm:
	cli 				; Turn off interrupts
	;xor ax, ax
	in al, 0x92			; enable a20
	or al, 2
	out 0x92, al

	xor ax, ax 			; Clear AX register
	mov ds, ax			; Set DS-register to 0 
	mov es, ax
	mov fs, ax
	mov gs, ax

	lgdt [gdt_desc] 	; Load the Global Descriptor Table

;==============================================================================
; ENTER PROTECTED MODE 

	mov eax, cr0
	or eax, 1               ; Set bit 0 
	mov cr0, eax

	jmp 08h:pm 				; Jump to code segment, offset kernel_segments


[BITS 32]
	pm:
	pop ecx					; End of memory map
	mov [mem_info+4], ecx 
	pop ecx
	mov [mem_info], ecx

	xor eax, eax

	mov ax, 10h 			; Save data segment identifyer
	mov ds, ax 				; Setup data segment
	mov es, ax
	mov ss, ax				; Setup stack segment
	mov fs, ax
	mov gs, ax

	mov esp, 0x00900000		; Move temp stack pointer to 090000h

	mov eax, vid_info
	push eax
	mov eax, mem_info
	push eax
	mov edx, 0x00050000
	lea eax, [edx]
	call eax				; stage2_main(mem_info, vid_info)

;==============================================================================
; GLOBAL DESCRIPTOR TABLE

align 16
mem_info:
	dd 0			
	dd 0



align 32
gdt:                            ; Address for the GDT

gdt_null:
	dd 0
	dd 0

;KERNEL_CODE equ $-gdt		; 0x08
gdt_kernel_code:
	dw 0FFFFh 	; Limit 0xFFFF
	dw 0		; Base 0:15
	db 0		; Base 16:23
	db 09Ah 	; Present, Ring 0, Code, Non-conforming, Readable
	db 0CFh		; Page-granular
	db 0 		; Base 24:31

;KERNEL_DATA equ $-gdt
gdt_kernel_data:                        
	dw 0FFFFh 	; Limit 0xFFFF
	dw 0		; Base 0:15
	db 0		; Base 16:23
	db 092h 	; Present, Ring 0, Code, Non-conforming, Readable
	db 0CFh		; Page-granular
	db 0 		; Base 24:31

gdt_end:                        ; Used to calculate the size of the GDT

gdt_desc:                       ; The GDT descriptor
	dw gdt_end - gdt - 1    ; Limit (size)
	dd gdt                  ; Address of the GDT
;==============================================================================

times 1024-($-$$) db 0

; We can use the end of the file for a convenient label
; Superblock starts at LBA 2, which is the end of this
; sector
superblock:
