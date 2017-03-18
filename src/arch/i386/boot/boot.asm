; set entry point to physical address
global _loader
global loader
loader equ (_loader - 0xC0000000)

; Juicy variables from paging.asm
extern initial_pd, initial_pd_real
extern kernel_vbase

; Our C kernel entry point
extern kernel_main

section .text
align 4

_loader:
    ; Load physical address of initial paging directory into cr3
    mov ecx, initial_pd_real
    mov cr3, ecx

    ; Set PSE bit in CR4 to enable 4MB pages.
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; Enabling paging in cr0
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    ; Long jump into the higher half
    lea ecx, [higherhalf]
    jmp ecx

higherhalf:
    ; Remove identity map for intial 4MB
    mov dword [initial_pd], 0
    invlpg [0]

    mov esp, stack_end

    mov ecx, initial_pd
    push ecx ; initial paging directory
    add ebx, kernel_vbase
    push ebx ; multiboot header
    push eax ; multiboot magic

    call  kernel_main
    jmp $

; Stack definitons
stack_size equ 0x4000
stack_end  equ stack_size + stack

section .bss
align 32
stack:
    resb stack_size
