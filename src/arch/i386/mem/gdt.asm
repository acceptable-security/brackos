global gdt_init
section .text

; struct to represent the gdt entries
struc gdt_entry
    limit_low:      resw 1
    base_low:       resw 1
    base_middle:    resb 1
    access:         resb 1
    granularity:    resb 1
    base_high:      resb 1
endstruc

; struct to represent the gdt itself
struc gdt_loc
    size:    resw 1
    address: resd 1
endstruc

; create_gdt_entry(base, limit, access, gran)
; macro for the creation of gdt entries based off of basic params
; everything is done at compile time :)
%macro create_gdt_entry 4
    istruc gdt_entry
        at limit_low,   dw %2 & 0xFFFF
        at base_low,    dw %1 & 0xFFFF
        at base_middle, db (%1 >> 16) & 0xFF
        at access,      db %3
        at granularity, db ((%2 >> 16) & 0x0F) | (%4 & 0xF0)
        at base_high,   db (%1 >> 24) & 0xFF
    iend
%endmacro

; our basic 3 gdt entries
; 1 - null entry
; 2 - kernel code segment
; 3 - kernel data segment
our_gdt_entries:
    create_gdt_entry 0, 0, 0, 0
    create_gdt_entry 0, 0xFFFFFFFF, 0x9A, 0xCF
    create_gdt_entry 0, 0xFFFFFFFF, 0x92, 0xCF
our_gdt_size equ $ - our_gdt_entries - 1

; our actual gdt object
; has the size - 1 and the address of our gdt entries
our_gdt:
    istruc  gdt_loc
        at size,    dw our_gdt_size
        at address, dd our_gdt_entries
    iend

; actual gdt loading code
; load the gdt from memory an then load the segment registers w/ a near jump.
gdt_init:
    push ebp
    mov ebp, esp
    mov eax, our_gdt
    ; sub eax, 0xC0000000
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:gdt_flush

gdt_flush:
    pop ebp
    ret
