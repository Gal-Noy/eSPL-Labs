section .data
    WRITE EQU 4
    READ EQU 3
    STDIN EQU 0
    STDOUT EQU 1
    STDERR EQU 2
    infile EQU STDIN
    outfile EQU STDOUT
    CHARLEN EQU 1
    newline db 10, 0
    buffer  db CHARLEN


section .text
    global _start
    extern strlen

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

encode:
    push    ebp             ; Save caller state
    mov     ebp, esp
    push    ebx
    
    mov     bl, [ebp+8]     ; Get the input character from the stack
    cmp     bl, 'a'         ; check if the character is a lowercase letter
    jb      not_a_to_z
    cmp     bl, 'z'
    ja      not_a_to_z
    add     bl, 1
    cmp     bl, 'z' + 1
    jne     encode_done
    mov     bl, 'a'
    jmp     encode_done
not_a_to_z:
    cmp     bl, 'A'         ; check if the character is a lowercase letter
    jb      encode_done
    cmp     bl, 'Z'
    ja      encode_done
    add     bl, 1
    cmp     bl, 'Z' + 1
    jne     encode_done
    mov     bl, 'A'
encode_done:
    mov     al, bl          ; place returned value where caller can see it
    pop     ebx
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

main:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    add     esi, 4          ; Start from argv[1]
    call    debug_loop
    call    encode_input

    popad                   ; Restore caller state (registers)
    mov     eax, 1          ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

debug_loop:
    ; Check if reached end of argv
    cmp     dword [esi], 0
    jge     debug_end

    ; Print next argument
    push    dword [esi]
    call    strlen
    add     esp, 4
    push    eax              ; Argument length
    push    dword [esi]
    push    STDERR
    push    WRITE
    call    system_call
    add     esp, 16

    ; Print new line
    push    CHARLEN
    push    newline
    push    STDERR
    push    WRITE
    call    system_call
    add     esp, 16

    ; Update registers
    add     esi, 4
    jmp      debug_loop
debug_end:
    ret

encode_input:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

io_loop:
    ; Read a character from input
    push    CHARLEN
    push    buffer
    push    infile
    push    READ
    call    system_call
    add     esp, 16

    ; Check if reached EOF
    cmp     eax, 0
    je      end_io_loop

    ; Encode character
    push    dword [buffer]
    call    encode
    add     esp, 4
    mov     [buffer], al

    ; Print encoded character to output
    push    CHARLEN
    push    buffer
    push    outfile
    push    WRITE
    call    system_call
    add     esp, 16

    jmp io_loop

end_io_loop:
    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller