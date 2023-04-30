section .data
    newline db 10, 0 ; define new line character
    lower_z db 'z', 0
    upper_z db 'Z', 0
    buffer  db 1
    WRITE EQU 4
    READ EQU 3
    STDIN EQU 0
    STDOUT EQU 1
    STDERR EQU 2
    infile EQU STDIN
    outfile EQU STDOUT
    CHARLEN EQU 1


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
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state
    
    ; Handle input
    mov     eax, [ebp+8]    ; Get the input character from the stack
    add     eax, 1          ; Increment character
    cmp     eax, lower_z
    jle     encode2
    sub     eax, 26
    jmp     encode_done
encode2:
    cmp     eax, upper_z
    jle     encode_done
    sub     eax, 26
encode_done:
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

main:
    ; Prolog
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    add     esi, 4          ; Start from argv[1]
    call    debug_loop
    call    encode_input

    ; Epilog
    popad                   ; Restore caller state (registers)
    mov     eax, 1    ; place returned value where caller can see it
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
    push    eax               ; Argument length
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

io_loop:
    ; Read a character from input
    push    CHARLEN
    push    buffer
    push    infile
    push    READ
    call    system_call
    add     esp, 16

    ; Check if reached EOF
    cmp     byte [buffer], 0
    je      end_io_loop

    push    CHARLEN
    push    buffer
    push    outfile
    push    WRITE
    call    system_call
    add     esp, 16

    

    ; ; Encode character
    ; call    encode

    ; ; Print encoded character to output
    ; push    CHARLEN
    ; push    esp
    ; push    outfile
    ; push    WRITE
    ; call    system_call
    ; add     esp, 16

    jmp io_loop
end_io_loop:
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller