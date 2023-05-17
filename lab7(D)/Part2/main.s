section .rodata
    x_struct: db 5
    x_num: db 0xaa,1,2,0x44,0x4f
    y_struct: db 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f

segment .text
global main
extern print_multi
extern getmulti
extern add_multi

 main:
    push    ebp                 
    mov     ebp, esp  

    ; push    dword x_struct
    ; call    print_multi

    ; call    getmulti
    ; push    dword eax
    ; call    print_multi

    push    dword y_struct
    push    dword x_struct
    call    add_multi
    ; push    dword eax
    ; call    print_multi

    mov     esp, ebp
    pop     ebp
    ret  