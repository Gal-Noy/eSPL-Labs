section .data
    newline db 10 ; define new line character

section .text
    global main
    extern strlen

main:
    mov edi, esp
    mov esi, [esp + 8] ; arg = argv[1]

loop_args:
    cmp edi, 1 ; check if only one arg left (program name)
    jmp end_loop

    ; print the argument
    push esi
    call strlen ; eax = arg length
    mov edx, eax
    mov eax, 4 ; the system call number for write(b=io, c=arg, d=len)
    mov ebx, 1 ; stdout
    mov ecx, esi ; arg to write
    mov edx, edx ; arg length
    int 0x80 ; make the system call

    ; print a newline
    mov eax, 4
    mov ebx, 1 ; stdout
    mov ecx, newline ; arg to write
    mov edx, 1 ; arg length
    int 0x80

    ; increment the pointer to argv and decrement the counter
    add esi, 4
    dec edi

    ; repeat the loop
    jmp loop_args
    
end_loop:
    mov eax, 1
    ; xor ebx, ebx
    int 0x80