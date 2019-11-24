[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000

	mov [BOOT_DRIVE], dl
	mov bp, 0x9000
	mov sp, bp
	mov bx, MSG_STARTING
	call print_string
	call load_kernel
	jmp KERNEL_OFFSET
	jmp $

%include "boot/print_string.asm"

load_kernel:
	pusha
	mov bx, MSG_LOAD_KERNEL
	call print_string
	mov bx, KERNEL_OFFSET
	mov dh, 30
	mov dl, [BOOT_DRIVE]
	call disk_load
	popa
	ret

; load DH sectors to ES:BX from drive DL
disk_load:
    push dx
    mov ah, 0x02
    mov al, dh
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13
    jc disk_error
    pop dx
    cmp dh, al
    jne disk_error
    ret
disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

BOOT_DRIVE db 0
MSG_STARTING db "Starting...", 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0
DISK_ERROR_MSG db "Disk read error!", 0

times 510-($-$$) db 0
dw 0xaa55
