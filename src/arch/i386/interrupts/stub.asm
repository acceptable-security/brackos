global irq_common_stub
extern irq_general_handler

irq_common_stub:
    cli
    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp

    call irq_general_handler

irq_load_state:
    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret
