;
; Ryan Jacoby <ryjacoby@calpoly.edu>
; fragaria/src/long_mode_start.asm
;
; 64-bit C wrapper
;

global long_mode_start
global panic
extern kmain

section .text
bits 64
long_mode_start:
        mov ax, 0                               ; zero all segment registers
        mov ss, ax
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov rax, 0x2f592f412f4b2f4f             ; print OKAY (now in 64 bits)
        mov qword [0x0b8000], rax

        pop rsi                                 ; put pointer in second arg
        pop rdi                                 ; put magic in first arg to
        call kmain                              ; jump to C

panic:
        mov rax, 0x4f444f414f454f44             ; print DEAD if kmain exits
        mov qword [0x0b8000], rax

.dead:
        hlt
        jmp .dead                               ; go back to sleep if we int
