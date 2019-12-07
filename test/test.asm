[bits 32]
[org 0x00200000]

; ptr to process heap is in ebx
;xchg ebx, ebx
mov ecx, 1024	;amount to request
mov eax, 1		;syscall alloc
int 0x64

;ptr to allocated mem in eax
mov [eax], byte 65
mov  [eax+1], byte 0
mov ebx, MSG_LOAD_KERNEL
mov eax, 2		;syscall print
int 0x64

mov eax, 3		;syscall exit
int 0x64

MSG_LOAD_KERNEL db "Loading kernel into memory", 0