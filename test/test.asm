[bits 32]
[org 0x00200000]

; ptr to process heap is in ebx
;xchg ebx, ebx
mov ecx, 1024	;amount to request
mov eax, 1		;syscall alloc
int 0x64


mov ecx, esp
mov ebx, MSG_ESP
push 4
pop eax
;mov eax, 4
int 0x64

mov eax, 3		;syscall exit
mov ebx, 0
int 0x64

MSG_LOAD_KERNEL db "Loading kernel into memory", 0
MSG_ESP db "esp", 0