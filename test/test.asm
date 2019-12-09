[bits 32]
[org 0x00200000]

start:
mov ecx, esp
mov ebx, MSG_THREAD
mov eax, 4
int 0x64
;jmp start

mov eax, 3		;syscall exit
mov ebx, 0
int 0x64

MSG_THREAD db "Thread ", 0