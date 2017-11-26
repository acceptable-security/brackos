global ap_boot_init
global ap_boot_end

extern our_gdt_phys
extern initial_pd, initial_pd_real
extern cpu_ready
extern ap_main

ap_stack_size equ 0x4000
ap_stack_end  equ ap_stack_size + ap_stack
ap_boot_pmode_nopaging_real equ ap_boot_pmode_nopaging - 0xC0000000

section .bss
align 32
ap_stack:
    resb ap_stack_size

section .text
[BITS 16]
ap_boot_init:
    ; Make sure interrupts are off
    cli

    ; Load the GDT
    mov eax, our_gdt_phys
    lgdt [eax]

    ; Enable the A20 gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Enable Protected Mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Far jump to protected mode
    jmp 0x08:dword ap_boot_pmode_nopaging_real

[BITS 32]
ap_boot_pmode_nopaging:
    mov esp, ap_stack_end

    ; Setup the segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

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
    lea ecx, [ap_boot_pmode_paging]
    jmp ecx

ap_boot_pmode_paging:
    call ap_main

    jmp $
    hlt

ap_boot_end:
ap_boot_size equ $ - ap_boot_init - 1
