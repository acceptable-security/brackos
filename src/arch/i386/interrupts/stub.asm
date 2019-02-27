global exception_stub0
global exception_stub1
global exception_stub2
global exception_stub3
global exception_stub4
global exception_stub5
global exception_stub6
global exception_stub7
global exception_stub8
global exception_stub9
global exception_stub10
global exception_stub11
global exception_stub12
global exception_stub13
global exception_stub14
global exception_stub15
global exception_stub16
global exception_stub17
global exception_stub18
global exception_stub19

global irq_common_stub
global irq_empty_stub
global irq_exit

extern irq_general_handler
extern exception_handle

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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call irq_general_handler
    pop eax

; This is used to handle interrupt cleanup but also preempting tasks.
irq_exit:
    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret

exception_stub0:
    cli
    push 0
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub1:
    cli
    push 1
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub2:
    cli
    push 2
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub3:
    cli
    push dword 3
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub4:
    cli
    push 4
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub5:
    cli
    push 5
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub6:
    cli
    push 6
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub7:
    cli
    push 7
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub8:
    cli
    push 8
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub9:
    cli
    push 9
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub10:
    cli
    push 10
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub11:
    cli
    push 11
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub12:
    cli
    push 12
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub13:
    cli
    push 13
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub14:
    cli
    push 14
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub15:
    cli
    push 15
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub16:
    cli
    push 16
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub17:
    cli
    push 17
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub18:
    cli
    push 18
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

exception_stub19:
    cli
    push 19
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

    ; Push the stack pointer with all those juicy registers.
    mov eax, esp
    push eax
    call exception_handle
    pop eax

irq_empty_stub: iret
