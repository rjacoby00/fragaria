section .multiboot_header
header_start:
        dd 0xE85250D6                   ; multiboot 2 magic number
        dd 0                            ; arch 0 (protected mode i386)
        dd header_end - header_start    ; header len
        ; checksum
        dd 0x100000000 - (0xE85250D6 + 0 + (header_end - header_start))

        ; optional multiboot tags

        ; end tag
        dw 0
        dw 0
        dd 8
header_end:
