section .data
    READ EQU 3
    WRITE EQU 4
    OPEN EQU 5
    CLOSE EQU 6
    STDIN EQU 0
    STDOUT EQU 1
    STDERR EQU 2
    CHARLEN EQU 1
    PERMISSIONS EQU 0644
    infile dd STDIN
    outfile dd STDOUT
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
main:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    call    handle_args
    call    encode_input

    ; Close infile and outfile
    mov     eax, CLOSE
    mov     ebx, dword[infile]
    int     0x80
    mov     eax, CLOSE
    mov     ebx, dword[outfile]
    int     0x80

    popad                   ; Restore caller state (registers)
    mov     eax, 1          ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

handle_args:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    add     esi, 4          ; Start from argv[1]
args_loop:
    ; Check if reached end of argv
    cmp     dword [esi], 0
    jge     args_end

    ; Print next argument
    push    dword [esi]
    call    strlen           ; Get argument length (=eax)
    add     esp, 4

    mov     edx, eax
    mov     ecx, dword [esi]
    mov     ebx, STDERR
    mov     eax, WRITE
    int     0x80
    
    ; Print new line
    mov     eax, WRITE
    mov     ebx, STDERR
    mov     ecx, newline
    mov     edx, CHARLEN
    int     0x80

  ; Check if the argument is in the form "-i{file}" or "-o{file}"
    mov     ebx, dword [esi]    ; Access the arg
    mov     al, byte [ebx]      ; Check first char (if '-')
    cmp     al, '-'
    jne     end_iteration

    inc     ebx                 ; Check first char (if 'i' or 'o')
    mov     al, byte [ebx]
    cmp     al, 'i'
    je      extract_infile
    cmp     al, 'o'
    je      extract_outfile
    jmp     end_iteration

extract_infile:
    inc     ebx                  ; Achieve file name
    mov     eax, OPEN
    mov     ecx, 0
    mov     edx, PERMISSIONS
    int     0x80
    mov     dword [infile], eax ; Update outfile
    jmp     end_iteration
extract_outfile:
    inc     ebx                  ; Achieve file name
    mov     eax, OPEN
    mov     ecx, 1
    mov     edx, PERMISSIONS
    int     0x80
    mov     dword [outfile], eax ; Update outfile
end_iteration:
    add     esi, 4               ; Move to next arg
    jmp     args_loop
args_end:
    popad                   ; Restore caller state (registers)
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
    cmp     bl, 'A'         ; check if the character is a uppercase letter
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
encode_input:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state
io_loop:
    ; Read a character from input
    mov     eax, READ
    mov     ebx, dword [infile]
    mov     ecx, buffer
    mov     edx, CHARLEN
    int     0x80

    ; Check if reached EOF
    cmp     eax, 0
    je      end_io_loop

    ; Encode character
    push    dword [buffer]
    call    encode
    add     esp, 4
    mov     [buffer], al

    ; Print encoded character to output
    mov     eax, WRITE
    mov     ebx, dword [outfile]
    mov     ecx, buffer
    mov     edx, CHARLEN
    int     0x80

    jmp io_loop
end_io_loop:
    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller