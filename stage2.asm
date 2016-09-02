[BITS 16]
[ORG 0x1000]


; To locate an inode: block group = (inode - 1) / s_inodes_per_group
; block table index = (inode-1) % s_inodes_per_group
; containing block = (bg->inode_table + (index * 128) / BLOCK_SIZE)
; INODE 5 is located in the first block table, index 0. + 4*128 = 0x200
; We need to access the s_inodes_per group, and inode_table fields
; Stage 3 bootloader is located in inode 5.

; We should enter this point (sector 2, block 1), with the next 4 sectors
; already read into data, in a contigous manner. So sector 3 starts at the
; end of this file: label superblock. 1024 bytes after that is the BG desc.

; We also need to convert LBA28 to CHS

start:


	xor ax, ax
	mov bx, ax
	mov dx, ax
	mov cx, ax

	mov ax, [superblock+56]				; EXT2_MAGUC
	cmp ax, 0xEF53						
	jne .error							; Not a valid EXT2 disk
	mov si, .ext2_success
	call .print

	mov ax, [superblock+40]				; s_inodes_per_group
	mov [inodes_per_group], ax			; store for later

	mov ax, [superblock + 1024 + 8]		; Block_Group_Descriptor->inode_table
	mov cx, 2							; Convert blocks to sectors
	mul cx								; ax = cx * ax
	add ax, 1							; CHS has index of 1

	mov [inode_five_block], ax
	mov [sector], ax
	xor ax, ax
	mov [cylinder], ax
	mov [head], ax

	mov ax, 2							; Read 2 sectors (1024 bytes)
	mov [sector_count], ax

	mov bx, 0						; Destination address
	;push bx

	call .read_disk

	mov [inode_five], bx
	mov dx, [inode_five + 28]
	mov cx, 4							; how many sectors to read?
	;call .read_loop
	;jmp .print
	call enter_pm

.read_loop:
	;mov si, .ext2_success
	;call .error
	sub cx, 2
	jz .loop_exit
	jmp .read_loop

.loop_exit:
	ret

.reset:
	mov		ah, 0					; reset floppy disk function
	mov		dl, 0					; drive 0 is floppy drive
	int		0x13					; call BIOS
	jc		.reset					; If Carry Flag (CF) is set, there was an error. Try resetting again
 
	mov		ax, 0x2000				; we are going to read sector to into address 0x1000:0
	mov		es, ax
	xor		bx, bx
.read_disk:

	;push bp
	mov ah, 0               ; Reset drive command
	int 13h                 ; Call interrupt 13h
	mov [drive], dl         ; Store boot disk
	or ah, ah               ; Check for error code
	jc .read_disk				; Try again if error

	mov ax, 0			; 0x2000:0
	mov es, ax				; ES segment = 0       

	mov bx, 0x0					; Destination address [es:bx]
	mov ah, 02h				; Read sector command
	mov al, 5	; Number of sectors to read (0x12 = 18 sectors)
	mov dl, [drive]         ; Load boot disk
	mov ch, 0               ; Cylinder = 0
	mov cl, 5              ; Starting Sector = 3
	mov dh, 1              ; Head = 1
	int 13h                 ; Call interrupt 13h
	or ah, ah               ; Check for error code
	jnz .error         ; Try again if error
	;pop bp
    ret


; mov si, msg
.print:
	lodsb					; load next byte from string from SI to AL
	or			al, al		; Does AL=0?
	jz			.printdone	; Yep, null terminator found-bail out
	mov			ah,	0eh		; Nope-Print the character
	int			10h
	jmp			.print		; Repeat until null terminator found
.printdone:
	ret					; we are done, so return


.error:
	mov si, .ext2_error
	call .print
	jmp $

.ext2_success	db 13, "EXT2 Loaded!", 0
.ext2_error		db 13, "EXT2 ERROR", 0

drive db 0
inode_five			dd 0	; store point to inode data
inodes_per_group 	dd 0	; store # inodes per group
inode_five_block 	dd 0	; store where inode 5 is
block_start			dd 0	; first data block for inode 5
sector_count			db 0	; how many data blocks to read
sector				db 0
head				db 0
cylinder			dw 0

enter_pm:
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

	jmp 08h:kernel_segments ; Jump to code segment, offset kernel_segments


[BITS 32]                       ; We now need 32-bit instructions
	kernel_segments:
	mov ax, 10h             ; Save data segment identifyer
	mov ds, ax              ; Setup data segment
	mov ss, ax              ; Setup stack segment
	;mov esp, 090000h        ; Move the stack pointer to 090000h

	mov eax, 0xB8000
	add eax, 20
	mov cx, 'Q'
	mov [eax], cx
	inc eax
	mov cx, 0x2
	mov [eax], cx



; 	mov edi, [0x80000 + 28h]   
; 	jmp $
; 	mov edx, edi
;    	mov eax, 0xFFFFFFFF

;    	add edx, 1024*720
;    	mov [edx], eax
;    	mov ecx, 0

; .loop:
	
;    	mov [edx+ecx], eax
;    	inc ecx

;    	cmp ecx, 1024*24
;    	jne .loop


	;jmp 08h:0x1000          ; Jump to section 08h (code), offset 01000h
	jmp $






	;----------Global Descriptor Table----------;

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


times 512-($-$$) db 0           ; Fill up the file with zeros
superblock:
