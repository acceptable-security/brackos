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
    uint16_t size;
    uint32_t base;
} __attribute__((packed)) idt_t;

typedef struct {
    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no, err_code;
    uint32_t eip;
    uint16_t cs;
    uint32_t eflags, useresp;
    uint16_t ss;
} irq_regs_t;

typedef struct {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} exc_regs_t;

void idt_set_gate(unsigned int gate, uintptr_t address, uint16_t selector, uint8_t gate_type);
void idt_load();
void idt_init();

#endif
