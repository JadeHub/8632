[bits 32]
[org 0x00500000]

start:
mov ecx, [0x00500000+0x400]
mov ebx, MSG_THREAD
mov eax, 4
int 0x64
jmp start

mov eax, 3		;syscall exit
mov ebx, 1
int 0x64

MSG_THREAD db "Thread ", 0