#include <stdint.h>

// Gate types
#define IDT_GATE_32_TASK      0x5
#define IDT_GATE_16_INTERRUPT 0x6
#define IDT_GATE_16_TRAP      0x7
#define IDT_GATE_32_INTERRUPT 0xE
#define IDT_GATE_32_TRAP      0xF

// Gate privileges
#define IDT_PRIV_KERN         0
#define IDT_PRIV_USER         3

// Maximum gate count.
#define IDT_GATE_COUNT        256

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t unused; // must be set to 0

        // Flags.
    uint8_t gate_type : 4;
    uint8_t storage_segment : 1;
    uint8_t descriptor_priv : 2;
    uint8_t present : 1;

    uint16_t offset_high;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t length;
    idt_gate_t gates[IDT_GATE_COUNT];
} __attribute__((packed)) idt_t;

void idt_set_gate(unsigned int gate, uintptr_t address, uint16_t selector, uint8_t gate_type);
void idt_load();
void idt_init();
