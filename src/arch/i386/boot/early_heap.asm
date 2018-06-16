global kernel_heap_size
global kernel_heap

section .bss

kernel_heap_size equ 0x6000

kernel_heap:
    resb kernel_heap_size