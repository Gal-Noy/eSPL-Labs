section .rodata
    x_struct: dd 5
    x_num: db 0xaa,1,2,0x44,0x4f   

segment .text
global main
extern print_multi
extern getmulti

 main:
    push    ebp                 
    mov     ebp, esp  

    push    dword x_struct
    call    print_multi
    ; call    getmulti

    mov     esp, ebp
    pop     ebp
    ret  