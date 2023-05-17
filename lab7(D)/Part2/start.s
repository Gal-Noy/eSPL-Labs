section .data
    BUFSIZE     EQU 600
    format:     db "%02hhx", 0
section .bss
    multi:      resb 1+BUFSIZE
    sum_multi:  resb 1+BUFSIZE
    buffer:     resb BUFSIZE

section .text
    global print_multi
    global getmulti
    global add_multi
    extern printf, fgets, stdin, malloc

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
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

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
    mov     byte [ebx], al      ; Store the pair of digits
    inc     ebx              
    inc     ecx                 ; Increment size by 1

    ; Move to the next pair in the buffer
    add     esi, 2              
    jmp     buffer_loop

    end_buffer_loop:
    mov     byte [edi], cl  ; Update multi size
    mov     [ebp-4], edi    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller


pair_to_hex:
    push    ebp             ; Save caller state
    mov     ebp, esp
    push    ebx
    push    ecx    

    mov     eax, [ebp+8]
    movzx   ebx, byte [eax] ; Get first character
    movzx   ecx, byte [eax+1] ; Get second character

    ; Convert first character to nibble value
    cmp     ebx, '0'                ; First digit is 0
    jb      second_char
    cmp     ebx, '9'                ; First digit is a letter
    ja      first_char_is_alpha
    sub     ebx, '0'                ; First digit is a number
    jmp     get_second_nibble

    first_char_is_alpha:
    cmp     ebx, 'a'
    jb      invalid_pair
    cmp     ebx, 'f'
    ja      invalid_pair
    sub     ebx, 'a' - 10           ; Adjust for alphabetical characters

    get_second_nibble:
    ; Convert second character to nibble value
    cmp     ecx, '0'                ; Second digit is 0
    jb      invalid_pair
    cmp     ecx, '9'                ; Second digit is a letter
    ja      second_char_is_alpha
    sub     ecx, '0'                ; Second digit is a number
    jmp     combine_nibbles

    second_char_is_alpha:
    cmp     ecx, 'a'
    jb      invalid_pair
    cmp     ecx, 'f'
    ja      invalid_pair
    sub     ecx, 'a' - 10           ; Adjust for alphabetical characters

    combine_nibbles:
    ; Combine nibbles to form byte
    shl     ebx, 4
    or      ebx, ecx
    mov     eax, ebx                ; Return result in al
    jmp end_convert

    invalid_pair:
    xor     eax, eax                ; Return 0 for invalid input
    jmp     end_convert

    second_char:
    xor     eax, eax                ; Return 0 for invalid input
    jmp     end_convert

    end_convert:
    pop     ecx
    pop     ebx
    pop     ebp
    ret                     ; Back to caller

get_max_min:
    push    ebp                 ; Save caller state
    mov     ebp, esp
    
    mov     eax, [ebp+8]        ; First multi pointer
    mov     ebx, [ebp+12]       ; Second multi pointer

    cmp     al, bl              ; Compare sizes
    jb      len2
    jmp     mm_end

    len2:
    mov     ebx, [ebp+8]        
    mov     eax, [ebp+12]       

    mm_end:
    pop     ebp                 ; Restore caller state
    ret                         ; Back to caller

add_multi:
    push    ebp                 ; Save caller state
    mov     ebp, esp
    sub     esp, 4              ; Leave space for local var on stack
    pushad                      ; Save some more caller state

    mov     edi, sum_multi
    push    dword [ebp+8]
    push    dword [ebp+12]
    call get_max_min
    add     esp, 8

    mov     ecx, ebx    
    mov     ebx, eax

    movzx   edx, byte [ebx]     ; Get the larger size
    inc_size:
    inc     edx
    mov     byte [edi], dl      ; Save buffer size

    push    ecx                 ; Save the value of ecx
    ; Allocate memory for num array
    push    edi
    call    malloc
    add     esp, 4
    pop     ecx                 ; Restore the value of ecx

    mov     dword [edi+1], eax  ; Store pointer in memory after size byte
    mov     edx, edi            ; Load pointer to new multi into edx

    ; Move pointers of multis to lsb
    movzx   eax, byte [ebx]
    add     ebx, eax
    movzx   eax, byte [ecx]
    add     ecx, eax
    movzx   eax, byte [edx]
    add     edx, eax

    movzx   esi, byte [edi]     ; Save sum size

sum_loop:
    ; Sum each corresponding digit
    mov     al, byte [ebx]
    add     al, byte [ecx]
; f:
    add     byte [edx], al

    ; Check if carry needs to be propagated
    cmp     byte [edx], 10
    jb      next_digit
    mov     al, byte [edx]
    sub     al, 10
    mov     byte [edx], al
    inc     byte [edx-1]

next_digit:
    ; Count digits
    dec     esi
    cmp     byte [esi], 0
    je      end_sum

    ; Move to next digit
    dec     ebx
    dec     ecx
    dec     edx
    jmp     sum_loop

end_sum:
    mov     [ebp-4], edi        ; Save returned value...
    popad                       ; Restore caller state (registers)
    mov     eax, [ebp-4]        ; place returned value where caller can see it
    add     esp, 4              ; Restore caller state
    pop     ebp                 ; Restore caller state
    ret                         ; Back to caller