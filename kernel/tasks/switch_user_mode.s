[GLOBAL switch_to_user_mode]
switch_to_user_mode:
    cli
   ; xchg bx, bx
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
    push long 0x1B ;CS
    push $cont      ;Address
    iret
cont:
    ret

