[GLOBAL switch_to_user_mode]
switch_to_user_mode:
    cli
    mov ax, 0x23    ; user data (gdt index 4)
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
    push long 0x1B	;CS 0x1B = user code (gdt index 3)
    push $cont      ;Address
    iret
cont:
    ret

[GLOBAL start_user_mode_thread]
start_user_mode_thread:
    cli
    mov ax, 0x23    ; user data (gdt index 4)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ebx, [esp+4]    ; addr    

    mov eax, esp
    push long 0x23  ;SS
    push eax        ;ESP
    pushf           ;Flags
    pop eax
    or eax, 0x200 ; enable interrupts
    push eax
    push long 0x1B	;CS 0x1B = user code (gdt index 3)
    push ebx      ; Address
    iret

[GLOBAL perform_task_switch]
perform_task_switch:
	mov edx, [esp+4]    ; esp ptr
    mov ecx, [esp+8]    ; ESP
    mov eax, [esp+12]   ; physical address of current directory
    
    ;  For cdecl; EAX, ECX, and EDX are already saved by the caller and dont need to be saved again
	push ebx            ; push the current state onto the stack
    push esi
    push edi
    push ebp

	mov [edx], esp      ; save the current esp
	mov esp, ecx        ; set new esp
	mov cr3, eax        ; set the page directory

	pop ebp             ; now pop off the new stack
    pop edi
    pop esi
    pop ebx
    ret   
	