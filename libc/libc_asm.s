[global perform_syscall:function]
perform_syscall:
; uint32_t perform_syscall(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5)
	push ebp
	mov ebp, esp
	mov edi, [esp+24]	;p5
	mov esi, [esp+20]	;p4
	mov edx, [esp+16]	;p3
	mov ecx, [esp+12]	;p2
	mov ebx, [esp+8]	;p1
	mov eax, [esp+4]	;id
	int 0x64
	leave
	ret