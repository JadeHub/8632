[global idt_flush:function]
idt_flush:
	mov eax, [esp+4]
	lidt [eax]
	ret

%macro ISR_NOERRCODE 1
	[global isr%1:function]
	isr%1:
	;	cli
		push byte 0
		push %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
	[global isr%1:function]
	isr%1:
		;cli
		push byte %1
		jmp isr_common_stub
%endmacro

[extern isr_handler]

[GLOBAL isr_common_stub:function]
isr_common_stub:
; isrXX has pushed err_code & int_num
; now we build the remainder of an isr_state_t
	pusha
	mov ax, ds
	push eax		; save ds

	; set selectors to kernel code
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; pass pointer to the isr_state_t as param
	push esp
	call isr_handler
	add esp, 4

	pop ebx			; restore ds
	mov ds, bx
	mov es, bx
	mov fs, bx
    mov gs, bx

	popa
	add esp, 8		; the two values pushed by isrXX (int_num, err_code)
	iret


ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE 30
ISR_NOERRCODE 31
ISR_NOERRCODE 100

; IRQs
ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 34
ISR_NOERRCODE 35
ISR_NOERRCODE 36
ISR_NOERRCODE 37
ISR_NOERRCODE 38
ISR_NOERRCODE 39
ISR_NOERRCODE 40
ISR_NOERRCODE 41
ISR_NOERRCODE 42
ISR_NOERRCODE 43
ISR_NOERRCODE 44
ISR_NOERRCODE 45
ISR_NOERRCODE 46
ISR_NOERRCODE 47
