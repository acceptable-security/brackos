global ap_boot
global ap_boot_size

extern our_gdt_phys
extern initial_pd, initial_pd_real
global ap_boot

ap_stack_size equ 0x4000
ap_stack_end  equ ap_stack_size + ap_stack

section .bss
align 32
ap_stack:
    resb ap_stack_size


[BITS 16]
section .text

ap_boot:
    ; Make sure interrupts are off
    cli

    ; Enable the A20 gate
    in al, 0x92
    or al, 0x02
    out 0x92, al

    mov esp, ap_stack_end

    ; Load the GDT
    mov eax, our_gdt_phys
    lgdt [eax]

    ; Enable Protected Mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Setup the things I forget the names of
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

[BITS 32]
    ; Far jump to protected mode
    jmp 0x08:.ap_boot_pmode_nopaging

.ap_boot_pmode_nopaging:
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
    lea ecx, [.ap_boot_pmode_paging]
    jmp ecx

.ap_boot_pmode_paging:
    jmp $
    hlt
ap_boot_size equ $ - ap_boot - 1
