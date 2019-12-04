[org 0x9000]

stage_two:
	mov bx, MSG_STARTING2
	call print_string	
    call mem_detect
	mov bx, 0x8500
	mov [bx], byte 'j'
    call switch_to_pm
	jmp $

MSG_STARTING2 db "Starting 2", 0

%include "boot/print_string.asm"
%include "boot/mem_detect.asm"
%include "boot/gdt.asm"
%include "boot/print_hex.asm"

%include "boot/switch_to_pm.asm"

[bits 32]

%include "boot/print_string_pm.asm"

BEGIN_PM:
	call clear_screen_pm
	mov ebx, MSG_PROT_MODE
	call print_string_pm
	jmp KENTRY_OFFSET
	jmp $

MSG_PROT_MODE db "32bit protected mode", 0

KENTRY_OFFSET: