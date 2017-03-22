; set entry point to physical address
global _loader
global loader
loader equ (_loader - 0xC0000000)

; Juicy variables from paging.asm
extern initial_pd, initial_pd_real
extern kernel_vbase

; Helpful information for the stack/heap
extern stack_end, kernel_heap, kernel_heap_size

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

    ; Add a recursive page directory entry.
    mov ecx, initial_pd_real
    or ecx, 3 ; read write & present
    mov dword [initial_pd + (1023 * 4)], ecx
    invlpg [0xFFFFF000]

    mov esp, stack_end

    mov ecx, kernel_heap_size
    push ecx ; kernel heap size
    mov ecx, kernel_heap
    push ecx ; kernel heap address
    mov ecx, initial_pd
    push ecx ; initial paging directory
    add ebx, kernel_vbase
    push ebx ; multiboot header
    push eax ; multiboot magic

    call  kernel_main
    jmp $
