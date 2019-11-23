print_string:
    pusha
    mov ah, 0x0e
print_string_loop:
    mov al, [bx]
    int 0x10
    add bx, 1
    cmp al, 0
    jne print_string_loop
	mov al, 10
	int 0x10
	mov al, 13
	int 0x10
    popa
    ret

	