global irq_common_stub
global irq_exit
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

; This is used to handle interrupt cleanup but also preempting tasks.
irq_exit:
    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret
