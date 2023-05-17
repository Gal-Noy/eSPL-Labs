section .rodata
    x_struct: db 1
    x_num: db 2
    y_struct: db 1
    y_num: db 3

segment .text
global main
extern print_multi
extern getmulti
extern add_multi

main:
    push    ebp                 
    mov     ebp, esp 

part0:
    ; push    dword x_struct
    ; call    print_multi

part1:
    ; call    getmulti
    ; push    dword eax
    ; call    print_multi

part2:
    push    dword y_struct
    push    dword x_struct
    call    add_multi

    ; ; Print new line
    ; push    newline
    ; call    printf
    ; add     esp, 4

    ; ; Print sum multi
    ; push    dword eax
    ; call    print_multi

    mov     esp, ebp
    pop     ebp
    ret