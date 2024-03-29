section .rodata
    x_struct: db 5
    x_num: db 0xaa, 1,2,0x44,0x4f
    y_struct: db 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f

section .data
    BUFSIZE     EQU 600
    format:     db "%02hhx", 0
    newline:    db 10, 0
    MASK        dw 0x1010       ; Mask for 16-bit Fibonacci LFSR
    STATE       dw 0xabcd       ; Initial state of the LFSR
section .bss
    xmulti:      resb 1+BUFSIZE
    ymulti:      resb 1+BUFSIZE
    buffer:     resb BUFSIZE

section .text
    global main
    global print_multi
    global getmulti
    global add_multi
    global rand_num
    global pr_multi
    extern printf, fgets, stdin, malloc, strlen

main:
    push    ebp                 
    mov     ebp, esp

    mov     ecx, [ebp+8]
    cmp     ecx, 1
    je      no_args

    mov     esi, [ebp+12]       ; esi = argv 
    add     esi, 4              ; esi = argv[1]

    mov     ebx, dword [esi]    ; Access the arg
    inc     ebx                 ; Check second char (if 'I' or 'R')
    mov     al, byte [ebx]
    cmp     al, 'I'
    je      i_arg

    r_arg:
    call    pr_multi
    push    dword eax
    call    pr_multi
    push    dword eax
    call    add_multi
    jmp     finish
    i_arg:
    call    getmulti
    push    dword eax
    call    getmulti
    push    dword eax
    call    add_multi
    jmp     finish
    no_args:
    push    dword y_struct
    push    dword x_struct
    call    add_multi

    finish:
    mov     esp, ebp
    pop     ebp
    ret

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

    mov     edi, xmulti
    cmp     byte [edi], 0
    je      init_pointers
    mov     edi, ymulti

    init_pointers:
    ; Initialize pointers
    mov     ebx, edi        ; ebx = p
    mov     ecx, 0          ; ecx = size = 0
    inc     ebx             ; ebx = num
    mov     esi, buffer     ; esi = buffer[0]

    buffer_loop:
    ; Check if we've reached the end of the buffer
    cmp     byte [esi], 0x0a    ; \n = reached the end of the buffer
    jbe      end_buffer_loop

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

    handle_odd:
    cmp     ecx, 10                 ; Only one byte
    jne     get_first
    mov     ecx, ebx
    mov     ebx, '0'

    get_first:
    ; Convert first character to nibble value
    cmp     ebx, '0'                ; First digit is 0
    jb      second_char
    cmp     ebx, '9'                ; First digit is a letter
    ja      first_char_is_alpha
    sub     ebx, '0'                ; First digit is a number
    jmp     get_second

    first_char_is_alpha:
    cmp     ebx, 'a'
    jb      invalid_pair
    cmp     ebx, 'f'
    ja      invalid_pair
    sub     ebx, 'a' - 10           ; Adjust for alphabetical characters

    get_second:
    ; Convert second character to nibble value
    cmp     ecx, '0'                ; Second digit is 0
    jb      invalid_pair
    cmp     ecx, '9'                ; Second digit is a letter
    ja      second_char_is_alpha
    sub     ecx, '0'                ; Second digit is a number
    jmp     combine

    second_char_is_alpha:
    cmp     ecx, 'a'
    jb      invalid_pair
    cmp     ecx, 'f'
    ja      invalid_pair
    sub     ecx, 'a' - 10           ; Adjust for alphabetical characters

    combine:
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
    pop     ecx
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
    h:
    movzx   edx, byte [eax]     ; Get the larger size
    inc     edx                 ; edx = sum multi size (add one for overflow)

    mov     ecx, ebx            ; ecx = smaller multi
    mov     ebx, eax            ; ebx = larger multi

    push    ecx
    push    edx

    ; Allocate memory for sum-multi
    mov     edi, edx            ; edi = edx
    push    edi
    call    malloc
    add     esp, 4

    pop     edx
    pop     ecx

    ; Save buffer size
    mov     byte [eax], dl      ; Save buffer size
    push    dword eax           ; Push sum-multi

    movzx   edi, byte [ecx]     ; Save small multi size
    movzx   esi, byte [ebx]     ; Save large multi size
    sub     esi, edi            ; esi = remainder

    ; Move pointers to start of buffers
    inc     eax                 ; sum-multi
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
    adc     byte [eax+1], 0     ; Move carry forward
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

    ; Propagate carry
    mov     dl, byte [ebx]
    adc     dl, byte [eax]      ; Add carry
    adc     byte [eax+1], 0     ; Move carry forward
    mov     byte [eax], dl

    ; Decrement loop counter and move pointers to next digit
    dec     esi
    inc     eax
    inc     ebx

    jmp     handle_longer

    end_sum:
    pop     eax                 ; Achieve the sum-multi

    ; Print the sum-multi
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
    push    ebp                 ; Save caller state
    mov     ebp, esp
    sub     esp, 4
    pushad

    movzx   eax, word [STATE]   ; Load state
    mov     ebx, eax
    
    and     eax, [MASK]         ; Get relevant bits
    jpe     even

    rcr     ebx, 1              ; Shift right with msb 1
    jmp     finish_rn
    even:
    shr     ebx, 1              ; Shift right with msb 0
    
    cmp     ebx, 0
    jne     finish_rn
    mov     ebx, 0xabcd

    finish_rn:
    mov     [STATE], ebx
    mov     [ebp-4], ebx
    g:
    popad
    mov     eax, [ebp-4]
    add     esp, 4
    pop     ebp                 ; Restore caller state
    ret                         ; Back to caller

pr_multi:
    push    ebp                     ; Save caller state
    mov     ebp, esp
    sub     esp, 4
    pushad

    generate_number:
    call    rand_num
    movzx   ebx, al                 ; Get n = size
    cmp     ebx, 0                  ; Check not zero
    je      generate_number

    mov     edi, ebx
    shl     edi, 3                ; For allocating 8*n bits
    inc     edi                     ; For size bit

    push    edi
    call    malloc
    add     esp, 4

    mov     edx, eax
    mov     byte [edx], bl          ; Save buffer size
    inc     edx                     ; Move to num buffer

    xor     ecx, ecx                ; Initialize counter

    generate_multi:
    cmp    ecx, ebx
    je     finish_pr
    inc    ecx

    call    rand_num                ; Generate a random byte

    mov     byte [edx], al          ; Store generated byte
    inc     edx

    jmp     generate_multi

    finish_pr:
    sub     edx, ebx
    dec     edx
    mov     [ebp-4], edx
    popad
    mov     eax, [ebp-4]
    add     esp, 4
    pop     ebp                     ; Restore caller state
    ret                             ; Back to caller


	
