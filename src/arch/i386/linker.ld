ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)

kern_mem = 0xC0000000;

SECTIONS {
    . = 0xC0100000;

    .text : AT(ADDR(.text) - kern_mem) {
        _multiboot = . - kern_mem;
        *(.multiboot)
        _code = . - kern_mem;
        *(.text)
        *(.rodata*)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - kern_mem) {
        _data = . - kern_mem;
        *(.data)
    }

    .bss : AT(ADDR(.bss) - kern_mem) {
        _bss = . - kern_mem;
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }

    physical_end = . - kern_mem; virtual_end = .;
}
