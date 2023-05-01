section .text
    global _start

_start:
    mov ebx, [esp+8]   ; get argv[1] pointer
f:
    cmp byte [ebx], '-' ; compare first char with '-'
    je exit_program      ; if not '-', print whole word

print_word:
    mov eax, [0]

exit_program:
    mov eax, 1          ; prepare to exit program
    xor ebx, ebx        ; exit status code 0
    int 0x80            ; invoke system call to exit
