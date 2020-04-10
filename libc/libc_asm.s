; uint32_t perform_syscall(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5)
[global perform_syscall:function]
perform_syscall:
	push ebp
	mov ebp, esp
	mov edi, [esp+28]	;p5
	mov esi, [esp+24]	;p4
	mov edx, [esp+20]	;p3
	mov ecx, [esp+16]	;p2
	mov ebx, [esp+12]	;p1
	mov eax, [esp+8]	;id
	int 0x64
	leave
	ret

; uint32_t regs_cs()
[global regs_cs:function]
regs_cs:
	mov eax, cs
	ret