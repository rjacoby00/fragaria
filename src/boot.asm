global start

section .text
bits 32
start:
        mov dword [0x0b8000], 0x2f4b2f4f        ; Write OK to VGA console
        hlt                                     ; Halt CPU
