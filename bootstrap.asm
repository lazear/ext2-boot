[BITS 16]                       ; We need 16-bit intructions for Real mode
[Org 0x7C00]
global entry
entry:

	mov bx, stage_oneandhalf
	mov [db_add], bx
	call read_disk

	jmp stage_oneandhalf


read_disk:
	mov si, PACKET		; address of "disk address packet"
	mov ah, 0x42		; AL is unused
	mov dl, 0x80		; drive number 0 (OR the drive # with 0x80)
	int 0x13
	jc error
	ret

; mov si, msg
; call print
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
	mov si, ext2_error
	call print
	ret


PACKET:
			db	0x10	; packet size (16 bytes)
			db	0		; always 0
blkcnt:		dw	4		; number of sectors to transfer
db_add:		dw	0	; memory buffer destination address (0:7c00)
			dw	0		; in memory page zero
d_lba:		dd	1		; put the lba to read in this spot
			dd	0		; more storage bytes only for big lba's ( > 4 bytes )


ext2_success	db 13, "EXT2 Magic Header Good!", 10, 0
ext2_error		db 13, "ERROR", 10, 0
in5_success		db 13, "Stage2 loaded!", 10, 0
stageonepointfive db 13, "Stage1.5 loaded!", 10, 0
protmode		db 13, "Entering PM!", 10, 0

block_count		dd 0
block_pointer	dd 0


times 510-($-$$) db 0           ; Fill up the file with zeros
dw 0AA55h                       ; Last 2 bytes = Boot sector identifyer

; LBA SECTOR 1. This is not loaded by BIOS. Stage1 loads Stage1.5

stage_oneandhalf:
	
	mov si, stageonepointfive
	call print

	xor bx, bx

	mov ax, [superblock+56]                         ; EXT2_MAGUC
	cmp ax, 0xEF53                                          
	jne error                                                      ; Not a valid EXT2 disk

	mov si, ext2_success
	call print

; We need to read the inode table entry for inode 5

	mov ax, [superblock + 1024 + 8]		; Block_Group_Descriptor->inode_table
	mov cx, 2							
	mul cx								; ax = cx * ax

	mov [d_lba], ax						; which sector do we read?

	mov ax, 2							; read 1024 bytes
	mov [blkcnt], ax	

	mov bx, 0x1000						; copy data to 0x1000
	mov [db_add], bx

	call read_disk

	xor bx, bx
	mov bx, 0x1200		; Inode 5 is 0x200 into the first block (index 4 * 128 bytes per inode)
	mov cx, [bx + 28]	; # of sectors for inode
	lea di, [bx + 40]	; address of first block pointer
	mov [block_count], cx

	mov bx, 0x5000
	mov [db_add+2], bx
	mov bx, 0		; where inode5 will be loaded. 0xF0000
	mov [db_add], bx
	call read_loop

	mov si, in5_success
	call print

	jmp enter_pm

; We enter the loop with:
;	bx = inode pointer
; 	cx = # of sectors to read (2 per block)
;	di = address of first block pointer
; No support for indirect pointers. So keep 
; stage2
read_loop:
	xor ax, ax		; clear ax
	mov dx, ax		; clear dx
	mov ax, [di]	; set ax = block pointer
	mov dx, 2		; multiply by 2 for sectors
	mul dx			; ax = dx * ax

	mov [d_lba], ax
	mov [db_add], bx

	call read_disk

	add bx, 0x400	; increase by 1 kb
	add di, 0x4		; move to next block pointer
	sub cx, 0x2		; reading two blocks
	jnz read_loop
	ret

	nop
	hlt

enter_pm:
	cli
	mov si, protmode	
	call print


	;xor ax, ax
	in al, 0x92				; enable a20
	or al, 2
	out 0x92, al

	xor ax, ax              ; Clear AX register
	mov ds, ax              ; Set DS-register to 0 
	mov es, ax
	mov fs, ax
	mov gs, ax

	lgdt [gdt_desc]         ; Load the Global Descriptor Table

	;----------Entering Protected Mode----------;

	mov eax, cr0            ; Copy the contents of CR0 into EAX
	or eax, 1               ; Set bit 0     (0xFE = Real Mode)
	mov cr0, eax            ; Copy the contents of EAX into CR0

	;jmp $
	;sti
	jmp 08h:pm ; Jump to code segment, offset kernel_segments


[BITS 32]                       ; We now need 32-bit instructions
	pm:
	mov ax, 10h             ; Save data segment identifyer
	mov ds, ax              ; Setup data segment
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax              ; Setup stack segment
	mov esp, 0x00900000        ; Move the stack pointer to 090000h

	mov eax, 0x000B8000
	add eax, 20
	mov cx, 'Q'
	mov [eax], cx

	;jmp 08h:0x1000          ; Jump to section 08h (code), offset 01000h
	jmp 0x00050000

	;----------Global Descriptor Table----------;

align 16
gdt:                            ; Address for the GDT

gdt_null:                       ; Null Segment
	dd 0
	dd 0


KERNEL_CODE             equ $-gdt		; 0x08
gdt_kernel_code:
	dw 0FFFFh               ; Limit 0xFFFF
	dw 0                    ; Base 0:15
	db 0                    ; Base 16:23
	db 09Ah                 ; Present, Ring 0, Code, Non-conforming, Readable
	db 0CFh                 ; Page-granular
	db 0                    ; Base 24:31

KERNEL_DATA             equ $-gdt		; 0x10
gdt_kernel_data:                        
	dw 0FFFFh               ; Limit 0xFFFF
	dw 0                    ; Base 0:15
	db 0                    ; Base 16:23
	db 092h                 ; Present, Ring 0, Data, Expand-up, Writable
	db 0CFh                 ; Page-granular
	db 0                    ; Base 24:32

gdt_end:                        ; Used to calculate the size of the GDT

gdt_desc:                       ; The GDT descriptor
	dw gdt_end - gdt - 1    ; Limit (size)
	dd gdt                  ; Address of the GDT


times 1020-($-$$) db 0
dd 0xDEADBEEF

; We can use the end of the file for a convenient label
; Superblock starts at LBA 2, which is the end of this
; sector
superblock:
