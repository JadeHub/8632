[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f
SCREEN_SIZE equ 2000

clear_screen_pm:
	pusha
	
	mov edx, VIDEO_MEMORY 			;address 
	mov ebx, 0						;char index (0 - 80*25)
	mov al, ' '
	mov ah, WHITE_ON_BLACK
clear_screen_pm_loop:
	cmp ebx, SCREEN_SIZE
	je clear_screen_pm_end	
	mov [edx], ax
	add ebx, 1
	add edx, 2
	jmp clear_screen_pm_loop
	
clear_screen_pm_end:	
	popa
	ret

print_string_pm:
	pusha
	mov edx, VIDEO_MEMORY

print_string_pm_loop:
	mov al, [ebx]
	mov ah, WHITE_ON_BLACK

	cmp al, 0
	je print_string_pm_done

	mov [edx], ax

	add ebx, 1
	add edx, 2

	jmp print_string_pm_loop

print_string_pm_done:
	popa
	ret
