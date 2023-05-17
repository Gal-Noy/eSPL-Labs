section .data
    BUFSIZE EQU 600
    format: db "%02hhx", 0
    dformat: db "%d", 10, 0
section .bss
    multi:  resb 2
    buffer: resb BUFSIZE

section .text
    global print_multi
    global getmulti
    extern printf, fgets, stdin

print_multi:
    push    ebp             ; Save caller state
    mov     ebp, esp

    mov     edi, [ebp+8]    ; edi = p
    mov     esi, [edi]      ; esi = size
    add     edi, 4
    
    mov     ebx, edi        ; pointer to num
    add     ebx, esi
    dec     ebx

chars_loop:
    ; Check if printed all chars
    cmp     esi, 0
    jz      loop_end

    push    dword [ebx]
    push    dword format
    call    printf
    add     esp, 8

    ; Move to next index
    dec     esi
    dec     ebx
    jmp     chars_loop

loop_end:
    mov     esp, ebp        ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

getmulti:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad

    ; ; read a line of input from stdin using fgets
    ; push    dword BUFSIZE
    ; push    dword buffer
    ; push    dword stdin
    ; call    fgets
    ; add     esp, 12

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
