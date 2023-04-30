section .data
    newline db 10, 0 ; define new line character
    WRITE db 4
    STDOUT db 1
    STDERR db 2

section .text
    global _start
    extern strlen

_start:
    pop     dword ecx ; ecx = argc
    mov     z
    mov edi, [esp + 8]
    push edi
    call strlen
    mov esi, eax

    mov eax, 4
    mov ebx, 1
    mov ecx, edi
    mov edx, esi
    int 0x80

    mov eax, 1
    mov ebx, 0
    int 0x80
