global kernel_heap_size
global kernel_heap

kernel_heap_size equ 0x6000

section .bss
kernel_heap:
    resb kernel_heap_size
