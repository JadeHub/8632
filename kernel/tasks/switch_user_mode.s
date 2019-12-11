[GLOBAL switch_to_user_mode]
switch_to_user_mode:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov eax, esp
    push long 0x23  ;SS
    push eax        ;ESP
    pushf           ;Flags
    ;enable interrupts
    pop eax
    or eax, 0x200
    push eax
    push long 0x1B	;CS
    push $cont      ;Address
    iret
cont:
    ret

[GLOBAL read_eip]
read_eip:
    pop eax                     ; Get the return address
    jmp eax

[GLOBAL perform_task_switch]
perform_task_switch:
	xchg bx, bx
	mov edx, [esp+8]  ; esp ptr
    mov eax, [esp+16]   ; physical address of current directory
    mov ecx, [esp+12]  ; ESP
	
	push ebx
    push esi
    push edi
    push ebp

	mov [edx], esp
	mov esp, ecx
	mov cr3, eax       ; set the page directory

	pop ebp
    pop edi
    pop esi
    pop ebx
 
	sti
    ret   
	

