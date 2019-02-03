#include <stdint.h>
#include <kernel/dwarf.h>
#include <stdlib.h>

// This was abandoned in favor of a ELF symbol table appraoch
// This code will be left here in the event that it becomes
// relevent in the future, however, it should be marked as
// ready for removal.
/* TODO: Remove the following code */

char* dwarf_find_source(dwarf_t* dwarf, uintptr_t address) {
    // uintptr_t start = (uintptr_t) &__debug_info_start;
    // uintptr_t end = (uintptr_t) &__debug_info_end;

    // notes from https://blog.tartanllama.xyz/writing-a-linux-debugger-elf-dwarf/
    // for each compile unit:
    // if the pc is between DW_AT_low_pc and DW_AT_high_pc:
    //     for each function in the compile unit:
    //         if the pc is between DW_AT_low_pc and DW_AT_high_pc:
    //             return function information

    // kprintf("line headres at %p\n", headers);

    // while ( start < end ) {
    //     dwarf_cu_header_t* cu_header = (dwarf_cu_header_t*) start;
    //     uintptr_t cu_end = ((uintptr_t) start) + cu_header->unit_length;
    //     uint8_t* die = (uint8_t*) (cu_header + 1);


    // }

    return NULL;
}