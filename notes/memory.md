# Address Space Map Plans
## Introduction
This is my first fleshed out memory map. Just putting things in as they come up and trying to make it as logical as
possible. Trying to get as many round number as possible :).

## Physical Memory Address Space
    --------------------+------------------------------------------------------------------------------------
    0x00000000          | Start of legacy memory
    0x0009FC00          | End of legacy memory (639KiB)
    0x000A0000          | End of real legacy memory (??) Start of BIOS memory
    0x00100000          | End of BIOS memory. Start of kernel memory.
    0x00106040          | End of preallocated kernel data. Start of kernel memory.
    0x01000000          | End of kernel memory.
    TODO
    0x07FE0000          | End of usable memory. Above this point it's all PCI and MMIO and Higher BIOS stuff
    --------------------+------------------------------------------------------------------------------------

## Virtual Memory Address Space
    --------------------+------------------------------------------------------------------------------------
    0x00000000          | NULL memory
    0x00001000          | user space start
    --------------------+------------------------------------------------------------------------------------
    0xC0000000          | Higher Half
    0xC00B8000          | VGA Memory
    0xC0100000          | Start of kernel memory (multiboot, text, rodata, data, bss)
    0xC0100000 + end    | End of preallocated kernel memory, and start of early kernel heap.
    end of kernel heap  | End of kernel heap, rest divided up into pages and used for later kernel heap
    0xD0000000          | End of kernel heal (255MiB). Start of page directory
    0xD0001000          | End of page directory. Start of page tables
    0xD0040000          | End of page tables. Rest is TODO
    --------------------+------------------------------------------------------------------------------------
