section .data
    format: db "%02hhx", 10, 0
    x_struct: db 5
    x_num: db 0xaa,1,2,0x44,0x4f

section .text
    global main
    extern printf

main:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    push    x_num
    push    x_struct

    call    print_multi

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    mov     eax,1
    int     0x80
    nop

print_multi:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    mov     ebx, [ebp+8]    ; ebx = p
    mov     esi, [ebx]      ; esi = size
    add     ebx, 4          ; ebx = num[0]

chars_loop:
    ; Check if printed all chars
    cmp     esi, 0
    jz      loop_end

    ; Print next char
    xor     ecx, ecx
    mov     cl, byte [ebx]

    push    ecx
    push    format
    call    printf
    add     esp, 8

    ; Move to next index
    inc     ebx
    dec     esi
    jmp     chars_loop

loop_end:
    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

