; Predefined variables
global kernel_vbase
global kernel_page
global initial_pd
global initial_pd_real

kernel_vbase    equ 0xC0000000
kernel_page     equ (kernel_vbase >> 22)

%define PAGE_DIR_PRESENT  1 << 0
%define PAGE_DIR_RW       1 << 1
%define PAGE_DIR_USER     1 << 2
%define PAGE_DIR_WT       1 << 3
%define PAGE_DIR_CACHE    1 << 4
%define PAGE_DIR_ACCESSED 1 << 5
%define PAGE_DIR_SIZE     1 << 7

%define PAGE(addr, flags) dd (addr & 0xfffff000) | flags

section .data
align 0x1000
initial_pd:
    PAGE(0, PAGE_DIR_SIZE | PAGE_DIR_RW | PAGE_DIR_PRESENT) ; Identity map kernel

    %rep (kernel_page - 1)
        PAGE(0, 0)
    %endrep

    PAGE(0, PAGE_DIR_SIZE | PAGE_DIR_RW | PAGE_DIR_PRESENT) ; Map kernel to 0xC0000000

    %rep (1024 - kernel_page - 1)
        PAGE(0, 0)
    %endrep

initial_pd_real equ initial_pd - kernel_vbase
