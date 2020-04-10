[GLOBAL switch_to_user_mode:function]
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

[GLOBAL start_user_mode_thread:function]
start_user_mode_thread:
    cli
    mov ax, 0x23    ; user data (gdt index 4)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ebx, [esp+4]    ; addr    

    mov eax, [esp+8]    ; esp
    push long 0x23  ;SS
    push eax        ;ESP
    pushf           ;Flags
    pop eax
    or eax, 0x200 ; enable interrupts
    push eax
    push long 0x1B	;CS 0x1B = user code (gdt index 3)
    push ebx      ; Address 
    mov ebp, 0x00 ;todo -hmm?
    iret

[GLOBAL start_kernel_mode_thread:function]
start_kernel_mode_thread:
    cli
    mov ax, 0x10    ; data (gdt index 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ebx, [esp+4]    ; addr    

    mov eax, esp
    push long 0x10  ;SS
    push eax        ;ESP
    pushf           ;Flags
    pop eax
    or eax, 0x200 ; enable interrupts
    push eax
    push long 0x08	;CS code (gdt index 1)
    push ebx      ; Address
    mov ebp, 0x00
    iret

[GLOBAL perform_task_switch:function]
perform_task_switch:
    mov edx, esp ; save the stack pointer so that we can obtain the params later
    
    ;  For cdecl; EAX, ECX, and EDX are already saved by the caller and dont need to be saved again
	push ebx            ; push the current state onto the stack
    push esi
    push edi
    push ebp

    mov ebx, [edx+16]   ; ebp_ptr
    mov [ebx], ebp

    mov ebx, [edx+4]   ; esp_ptr
    mov [ebx], esp      ; save the current esp
	mov esp, [edx+8]        ; set new esp
    mov eax, [edx+12]   ; cant move from mem to cr3
	mov cr3, eax        ; set the page directory

	pop ebp             ; now pop off the new stack
    pop edi
    pop esi
    pop ebx
    ret   
	