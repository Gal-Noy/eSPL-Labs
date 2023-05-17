section .data
    BUFSIZE     EQU 600
    format:     db "%02hhx", 0
    newline:    db 10, 0
    MASK        dw 0b1000000000000001   ; Mask for 16-bit Fibonacci LFSR
    STATE       dw 12345                ; Initial state of the LFSR
section .bss
    multi:      resb 1+BUFSIZE
    buffer:     resb BUFSIZE

section .text
    global main
    global print_multi
    global getmulti
    global add_multi
    global rand_num
    extern printf, fgets, stdin, malloc

print_multi:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad
    mov     edi, [ebp+8]        ; edi = p
    movzx   ebx, byte [edi]     ; ebx = size
    add     edi, ebx            ; edi = last num

remove_leading_zeros:
    cmp     byte [edi], 0
    jne     chars_loop
    dec     ebx
    dec     edi
    jmp remove_leading_zeros

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
    push    ecx

    mov     eax, [ebp+8]        ; First multi pointer
    mov     ebx, [ebp+12]       ; Second multi pointer

    mov     cl, byte [eax]
    cmp     cl, byte [ebx]      ; Compare sizes
    jb      len2
    jmp     mm_end

    len2:
    mov     eax, [ebp+12]      
    mov     ebx, [ebp+8]        

    mm_end:
    pop ecx
    pop     ebp                 ; Restore caller state
    ret                         ; Back to caller

add_multi:
    push    ebp                 ; Save caller state
    mov     ebp, esp
    sub     esp, 4
    pushad                      ; Save some more caller state

    ; Print multis with \n characters
    push    dword [ebp+8]
    call    print_multi
    add     esp, 4
    push    newline
    call    printf
    add     esp, 4
    push    dword [ebp+12]
    call    print_multi
    add     esp, 4
    push    newline
    call    printf
    add     esp, 4

    ; Find multi with larger size
    push    dword [ebp+8]
    push    dword [ebp+12]
    call    get_max_min
    add     esp, 8

    movzx   edx, byte [eax]     ; Get the larger size
    inc     edx                 ; edx = sum multi size

    mov     ecx, ebx            ; ecx = smaller multi
    mov     ebx, eax            ; ebx = larger multi

    push    ecx
    push    edx

    ; Allocate memory for sum multi
    mov     edi, edx            ; edi = edx
    push    edi
    call    malloc
    add     esp, 4

    pop     edx
    pop     ecx

    ; Save buffer size
    mov     byte [eax], dl      ; Save buffer size
    push    dword eax           ; Push sum multi

    movzx   edi, byte [ecx]     ; Save small multi size
    movzx   esi, byte [ebx]     ; Save large multi size
    sub     esi, edi            ; esi = remainder

    ; Move pointers to start of buffers
    inc     eax                 ; Sum multi
    inc     ebx                 ; Large multi
    inc     ecx                 ; Small multi
    xor     edx, edx            ; Clear edx
    CLC                         ; Clear carry flag

sum_loop:
    cmp     edi, 0              ; Finished with small multi?
    je      handle_longer

    ; Sum digits
    mov     dl, byte [ebx]
    adc     dl, byte [ecx]
    adc     byte [eax+1], 0
    add     byte [eax], dl

    ; Decrement loop counter and move pointers to next digit
    dec     edi
    inc     eax
    inc     ebx
    inc     ecx

    jmp     sum_loop

handle_longer:
    cmp     esi, 0              ; Finished with small multi?
    je      end_sum

    mov     dl, byte [ebx]
    adc     dl, 0
    mov     byte [eax], dl

    ; Decrement loop counter and move pointers to next digit
    dec     esi
    inc     eax
    inc     ebx

    jmp     handle_longer

end_sum:
    pop     eax                 ; Achieve the sum multi

    ; Print the sum multi
    push    dword eax
    call    print_multi
    add     esp, 4

    mov     [ebp-4], eax        ; Save returned value...
    popad                       ; Restore caller state (registers)
    mov     eax, [ebp-4]        ; place returned value where caller can see it
    add     esp, 4
    pop     ebp                 ; Restore caller state
    ret                         ; Back to caller

rand_num:
    ; Load state and mask into registers
    mov     ecx, MASK
    mov     edx, STATE

    ; Compute parity of relevant bits using bit manipulation
    mov     eax, edx
    and     eax, ecx   ; Select relevant bits
    xor     eax, edx
    xor     eax, ecx
    xor     eax, edx
    xor     eax, ecx
    shr     eax, 15    ; Extract the parity bit

    ; Shift state one position to the right and insert new bit
    shr     edx, 1
    rcl     edx, 1, eax
    
    ; Update global state variable
    mov     STATE, edx
    
    ; Return pseudo-random number in AX
    mov     ax, dx
    ret




	
