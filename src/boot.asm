global start
extern long_mode_start                          ; Entry point of long mode

global pf_stack_top
global pf_stack_bottom
global df_stack_top
global df_stack_bottom
global gp_stack_top
global gp_stack_bottom

section .text
bits 32
start:
        mov esp, stack_top                      ; Init stack

        call check_multiboot                    ; Perform checks
        call check_cpuid
        call check_long_mode

        call set_up_page_tables                 ; Set up identity map
        call enable_paging

        lgdt [gdt64.pointer]                    ; Load 64 bit GDT

        jmp gdt64.code:long_mode_start          ; Far jump to long mode using
                                                ; gdt64.code code descriptor

check_multiboot:
        cmp eax, 0x36d76289                     ; Check multiboot magic
        jne .no_multiboot
        ret
.no_multiboot:
        mov al, "0"                             ; If no multiboot, err 0
        jmp error


check_cpuid:
        ; If CPUID is supported, we can flip bit 21 in FLAGS
        pushfd
        pop eax                                 ; Copy FLAGS to EAX
        mov ecx, eax
        xor eax, 1<<21                          ; Try flipping bit 21
        push eax                                ; Write back to FLAGS
        popfd

        pushfd                                  ; Copy it back to EAX
        pop eax

        push ecx                                ; Revert FLAGS to orignal
        popfd

        cmp eax, ecx                            ; Check if bit was flipped
        je .no_cpuid
        ret
.no_cpuid:
        mov al, "1"                             ; If no CPUID, err 1
        jmp error


check_long_mode:
        mov eax, 0x80000000
        cpuid
        cmp eax, 0x80000001                     ; Test CPU age
        jb .no_long_mode                        ; If new enough, check LM

        mov eax, 0x80000001
        cpuid
        test edx, 1<<29                         ; Test long mode bit
        jz .no_long_mode
        ret
.no_long_mode:
        mov al, "2"                             ; If no long mode, err 2
        jmp error


set_up_page_tables:
        mov eax, p3_table                       ; map first P4 entry to P3
        or eax, 0b11                            ; Set present+writable
        mov [p4_table], eax                     ; write to first entry

        mov eax, p2_table                       ; map first P3 entry to P2
        or eax, 0b11
        mov [p3_table], eax

        mov ecx, 0                              ; init loop counter
.map_p2_table:
        mov eax, 0x200000                       ; Table size 2 MiB
        mul ecx
        or eax, 0b10000011                      ; Set present, writable, huge
        mov [p2_table + ecx * 8], eax

        inc ecx
        cmp ecx, 512
        jl .map_p2_table                       ; if count<512, loop again

        ret


enable_paging:
        mov eax, p4_table                       ; load P4 into CR3
        mov cr3, eax

        mov eax, cr4                            ; enable PAE-flag in CR4
        or eax, 1<<5
        mov cr4, eax

        mov ecx, 0xC0000080                     ; set long mode bit in MSR
        rdmsr
        or eax, 1<<8
        wrmsr

        mov eax, cr0                            ; enable paging in CR0
        or eax, 1<<31
        mov cr0, eax

        ret


; Print ERR and code to screen and halt
; Pass error code in al
error:
        mov dword [0x0b8000], 0x4f524f45
        mov dword [0x0b8004], 0x4f3a4f52
        mov dword [0x0b8008], 0x4f204f20
        mov byte  [0x0b800a], al
        hlt


section .bss
align 4096
p4_table:
        resb 4096
p3_table:
        resb 4096
p2_table:
        resb 4096
stack_bottom:
        resb 2048
stack_top:
pf_stack_bottom:
        resb 512
pf_stack_top:
df_stack_bottom:
        resb 512
df_stack_top:
gp_stack_bottom:
        resb 512
gp_stack_top:

section .rodata
gdt64:
        dq 0
.code:  equ $ - gdt64
        dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
.pointer:
        dw $ - gdt64 - 1
        dq gdt64
