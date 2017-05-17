#ifndef _IDT_H
#define _IDT_H

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

    uint8_t flags;

    uint16_t offset_high;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t length;
    uint32_t base;
} __attribute__((packed)) idt_t;

typedef struct {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} idt_reg_stack_t;


void idt_set_gate(unsigned int gate, uintptr_t address, uint16_t selector, uint8_t gate_type);
void idt_load();
void idt_init();

#endif
