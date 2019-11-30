[global gdt_flush]
gdt_flush:
	mov eax, [esp+4]
	lgdt [eax]
	ret

[global tss_flush]
tss_flush:
	; TSS is 5th item in GDT = 0x28
	; set RPL of 3 making 0x2B
	mov ax, 0x2B
   	ltr ax
   	ret