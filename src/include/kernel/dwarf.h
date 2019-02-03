#include <stdint.h>

typedef struct {
    uint32_t length;
    uint16_t version;
    uint32_t header_length;
    uint8_t min_instruction_length;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
    uint8_t std_opcode_lengths[12];
} __attribute__((packed)) dwarf_debug_line_header_t;

typedef struct {
    uint32_t unit_length;
    uint16_t version;
    uint32_t debug_abbrev_offset;
    uint8_t address_size;
} __attribute__((packed)) dwarf_cu_header_t;

typedef struct {
        dwarf_debug_line_header_t* debug_line;
        dwarf_cu_header_t* debug_info;
        
} dwarf_t;