; uint32_t regs_ebp()
[global regs_ebp:function]
regs_ebp:
	mov eax, ebp
	ret

; uint32_t regs_eip()
[global regs_eip:function]
regs_eip:
	mov eax, [esp]
	ret

; uint32_t regs_esp()
[global regs_esp:function]
regs_esp:
	mov eax, esp
	sub eax, 4
	ret

