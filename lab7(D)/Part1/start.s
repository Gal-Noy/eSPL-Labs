section .data
    BUFSIZE     EQU 600
    format:     db "%02hhx", 0
section .bss
    multi:      resb 2+BUFSIZE
    buffer:     resb BUFSIZE

section .text
    global print_multi
    global getmulti
    extern printf, fgets, stdin

print_multi:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad

    mov     edi, [ebp+8]        ; edi = p
    movzx   ebx, byte [edi]     ; ebx = size
    add     edi, ebx            ; edi = last num

chars_loop:
    ; Check if printed all chars
    cmp     ebx, 0
    jz      loop_end

    push    dword [edi]
    push    dword format
    call    printf
    add     esp, 8

    ; Move to next index
    dec     ebx
    dec     edi
    jmp     chars_loop

loop_end:
    popad
    pop     ebp
    ret                     ; Back to caller

getmulti:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad
    
    ; read a line of input from stdin using fgets
    push    dword [stdin]   ; stream
    push    dword BUFSIZE
    push    dword buffer
    call    fgets
    add     esp, 12

    ; Initialize pointers
    mov     edi, multi
    mov     ebx, edi        ; ebx = p
    mov     ecx, 0          ; ecx = size = 0
    inc     ebx             ; ebx = num
    mov     esi, buffer     ; esi = buffer[0]

buffer_loop:
    ; Check if we've reached the end of the buffer
    cmp     byte [esi], 0x0a    ; \n = reached the end of the buffer
    je      end_buffer_loop

    ; Convert the current pair of characters to hex
    push    dword esi           
    call    pair_to_hex         ; Convert first two digits to hex
    add     esp, 4

    ; Store the result in the multi struct
    mov     edx, [eax]
    mov     word [ebx], dx      ; Store the pair of digits
    add     ebx, 2              
    inc     ecx                 ; Increment size by 1

    add     esi, 2              ; Move to the next pair in the buffer
    jmp     buffer_loop

end_buffer_loop:
    mov     byte [edi], cl  ; Update multi size
f:
    popad
    pop     ebp
    mov     eax, edi        ; Return the address of the multi struct
    ret                     ; Back to caller

pair_to_hex:
    push    ebp             ; Save caller state
    mov     ebp, esp
    push    ebx

    mov     bx, word [ebp+8]

    ; ; Convert the first byte to hex
    ; mov     al, bl
    ; shr     al, 4
    ; call    byte_to_hex
    ; mov     bl, al
    mov     ax, bx
    pop     ebx
    pop     ebp
    ret                     ; Back to caller