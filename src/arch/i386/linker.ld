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
    }

    /* Thank you to Andrew Kelley for this trick
     * from his blog post on andrewkelley.me
     * titled " Using Zig to Provide Stack Traces
     * on Kernel Panic for a Bare Bones Operating System
     */
    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - kern_mem) {
        *(.rodata)
        __debug_info_start = .;
        KEEP(*(.debug_info))
        __debug_info_end = .;
        __debug_abbrev_start = .;
        KEEP(*(.debug_abbrev))
        __debug_abbrev_end = .;
        __debug_str_start = .;
        KEEP(*(.debug_str))
        __debug_str_end = .;
        __debug_line_start = .;
        KEEP(*(.debug_line))
        __debug_line_end = .;
        __debug_ranges_start = .;
        KEEP(*(.debug_ranges))
        __debug_ranges_end = .;
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

    physical_end = . - kern_mem;
    virtual_end = .;
}
