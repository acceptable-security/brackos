global gdt_init
global our_gdt
global our_gdt_phys
global tss_set
section .text

; struct to represent the gdt entries
struc gdt_entry
    limit_low:      resw 1 ; 0, 1
    base_low:       resw 1 ; 2, 3
    base_middle:    resb 1 ; 4
    access:         resb 1 ; 5
    granularity:    resb 1 ; 6
    base_high:      resb 1 ; 7
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

; 1 - null entry
; 2 - kernel code segment
; 3 - kernel data segment
; 4 - user code segment
; 5 - user data segment
; 6 - tss entry
our_gdt_entries:
    create_gdt_entry 0, 0, 0, 0
    create_gdt_entry 0, 0xFFFFFFFF, 0x9A, 0xCF
    create_gdt_entry 0, 0xFFFFFFFF, 0x92, 0xCF
    create_gdt_entry 0, 0xFFFFFFFF, 0xFA, 0xCF
    create_gdt_entry 0, 0xFFFFFFFF, 0xF2, 0xCF
tss_entry:
    create_gdt_entry 0, 0x67,       0x89, 0xCF ; Empty descriptor for TSS
our_gdt_size equ $ - our_gdt_entries - 1

; our actual gdt object
; has the size - 1 and the address of our gdt entries
our_gdt:
    istruc  gdt_loc
        at size,    dw our_gdt_size
        at address, dd our_gdt_entries
    iend

our_gdt_phys equ our_gdt - 0xC0000000

; actual gdt loading code
; load the gdt from memory an then load the segment registers w/ a near jump.
gdt_init:
    push ebp
    mov ebp, esp
    mov eax, our_gdt
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

; Set TSS
tss_set:
    push ebp
    mov ebp, esp

    push eax
    push ebx
    push ecx

    mov eax, [ebp + 8] ; Get the address off the stack

    ; ECX contains the low limit
    mov ecx, eax
    and ecx, 0xFFFFF

    ; EBX contains the middle byte
    mov ebx, eax
    shr ebx, 16
    and ebx, 0xFF

    ; EAX contains the high limit
    shr eax, 24
    and eax, 0xFF

    mov word [tss_entry + 2], cx ; Load low limit
    mov byte [tss_entry + 4], bl ; Load middle limit
    mov byte [tss_entry + 7], al ; Load high limit

    ; Reload GDT
    lgdt [our_gdt]

    ; Flush GDT
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:tss_gdt_flush
tss_gdt_flush:

    ; Load the actual TSS
    mov ax, 0x28 ; 6th entry
    ltr ax

    pop ecx
    pop ebx
    pop eax

    pop ebp
    ret
