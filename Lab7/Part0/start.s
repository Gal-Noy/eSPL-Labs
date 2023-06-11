section .data
    format db "%d", 10, 0
section .text
    global main
    extern puts
    extern printf

main:
    ; Print argc
    push    dword [esp+4]
    push    format
    call    printf
    add     esp, 8

    mov     ebx, dword [esp+8]      ; ebx = argv[0]

argv_loop:
    ; Check if reached end of argv
    cmp     dword [ebx], 0
    je     loop_end

    ; Print next argument with puts
    push    dword [ebx]
    call    puts
    add     esp, 4

    ; Move to next arg
    add     ebx, 4
    jmp     argv_loop   

loop_end:
    mov     eax,1
    int     0x80
    nop