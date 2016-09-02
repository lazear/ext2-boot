; Fat12 Bootloader

[BITS 16]                       ; We need 16-bit intructions for Real mode

;[ORG 0x7C00]                    ; The BIOS loads the boot sector into memory location 0x7C00

;jmp word read_disk            ; Load the OS Kernel
global read_disk
read_disk:
        mov ah, 0               ; Reset drive command
        int 13h                 ; Call interrupt 13h
        mov [drive], dl         ; Store boot disk
        or ah, ah               ; Check for error code
        jnz .error           ; Try again if error
        
        mov ax, 0               ; Clear AX
        mov es, ax              ; ES segment = 0                
        mov bx, 0x1000          ; Destination address = 0000:1000
        mov ah, 02h             ; Read sector command
        mov al, 10               ; Number of sectors to read (0x12 = 18 sectors)
        mov dl, [drive]         ; Load boot disk
        mov ch, 0               ; Cylinder = 0
        mov cl, 2               ; Starting Sector = 3
        mov dh, 0               ; Head = 1
        int 13h                 ; Call interrupt 13h
        or ah, ah               ; Check for error code
        jnz .error         ; Try again if error

        cli
        xor ax, ax
        mov ds, ax
        mov es, ax
        mov ss, ax
        jmp enter_pm

.error:
        mov ah, 09h
        mov al, '!'
        mov bh, 0
        mov bl, 0xA;
        mov cx, 160*25
        int 0x10
        jmp $

enter_pm:
        in al, 0x92                             ; enable a20
        or al, 2
        out 0x92, al

        xor ax, ax              ; Clear AX register
        mov ds, ax              ; Set DS-register to 0 

        lgdt [gdt_desc]         ; Load the Global Descriptor Table

        ;----------Entering Protected Mode----------;

        mov eax, cr0            ; Copy the contents of CR0 into EAX
        or eax, 1               ; Set bit 0     (0xFE = Real Mode)
        mov cr0, eax            ; Copy the contents of EAX into CR0

        jmp 08h:pm ;            Jump to code segment, offset kernel_segments
 

[BITS 32]                       ; We now need 32-bit instructions
pm:
        mov ax, 10h             ; Save data segment identifyer
        mov ds, ax              ; Setup data segment
        mov ss, ax              ; Setup stack segment
        ;mov esp, 090000h        ; Move the stack pointer to 090000h
        mov eax, 0xB8000
        mov ecx, 'A'
        mov [eax], ecx
        jmp 08h:0x1000          ; Jump to STAGE 2


;----------Global Descriptor Table----------;

gdt:                            ; Address for the GDT

gdt_null:                       ; Null Segment
        dd 0
        dd 0


KERNEL_CODE             equ $-gdt               ; 0x08
gdt_kernel_code:
        dw 0FFFFh               ; Limit 0xFFFF
        dw 0                    ; Base 0:15
        db 0                    ; Base 16:23
        db 09Ah                 ; Present, Ring 0, Code, Non-conforming, Readable
        db 0CFh                 ; Page-granular
        db 0                    ; Base 24:31

KERNEL_DATA             equ $-gdt               ; 0x10
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
      

drive db 0
times 510-($-$$) db 0           ; Fill up the file with zeros
dw 0AA55h                       ; Last 2 bytes = Boot sector identifyer
