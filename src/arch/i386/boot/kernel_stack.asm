global stack_size, stack_end

; Stack definitons
stack_size equ 0x2000

section .bss
align 32
stack:
    resb stack_size

stack_end  equ stack_size + stack
