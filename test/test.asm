[org 0x00200000]

; ptr to process heap is in ebx
mov ecx, $1024	;amount to request
mov eax, $1		;syscall alloc
int $100

;ptr to allocated mem in eax
mov [eax], byte $65
mov  [eax+1], byte $0
mov ebx, eax
mov eax, $1		;syscall print
int $100

mov eax, $3		;syscall exit
int $100