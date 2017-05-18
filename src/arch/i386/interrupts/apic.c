#include <arch/i386/msr.h>
#include <arch/i386/io.h>
#include <arch/i386/paging.h>
#include <mem/vasa.h>
#include <stdlib.h>
#include <stdbool.h>
#include <kprint.h>

#define APIC_REG_ID                  0x0020
#define APIC_REG_VERSION             0x0030
#define APIC_REG_TASK_PRIORITY       0x0080
#define APIC_REG_END_OF_INTERRUPT    0x00B0
#define APIC_REG_LOGICAL_DESTINATION 0x00D0
#define APIC_REG_DESTINATION_FORMAT  0x00E0
#define APIC_REG_SPURIOUS_INTERRUPT  0x00F0
#define APIC_REG_INTERRUPT_CMD_LOW   0x0300
#define APIC_REG_INTERRUPT_CMD_HIGH  0x0310
#define APIC_REG_LVT_TIMER           0x0320
#define APIC_REG_LVT_PERF_COUNTER    0x0340
#define APIC_REG_LVT_LINT0           0x0350
#define APIC_REG_LVT_LINT1           0x0360
#define APIC_REG_LVT_ERROR           0x0370
#define APIC_TIMER_INITIAL_COUNT     0x0380
#define APIC_TIMER_CURRENT_COUNT     0x0390
#define APIC_TIMER_DIVIDER           0x03E0

uintptr_t apic_base;

// Return bool if the APIC is supported
bool apic_supported() {
    unsigned int _unused, edx;
    int success = __get_cpuid(0x01, &_unused, &_unused, &_unused, &edx);

    if ( success == 0 ) {
        kprintf("cpuid failed\n");
        return false;
    }

    return (edx & bit_APIC) != 0;
}

// Set the physical address of the APIC
void apic_set_base(uintptr_t apic) {
    uint32_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
    cpu_set_msr(IA32_APIC_BASE_MSR, eax, 0);
}

// Return the physical address of the APIC
uintptr_t apic_get_base() {
    uint32_t eax, edx;
    cpu_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);
    return (eax & 0xfffff000);
}

void apic_register_writel(unsigned long reg, uint32_t value) {
    *(uint32_t*)(apic_base + reg) = value;
}

void apic_eoi() {
    apic_register_writel(APIC_REG_END_OF_INTERRUPT, 0x00000000);
}

bool apic_is_enabled() {
    return (inportb(0xF0) & 0x100);
}

void apic_enable() {
    // Hardware enable APIC
    uintptr_t base = apic_get_base();
    apic_set_base(base);

    void* virt = vasa_alloc(MEM_PCI, 4096, 0);

    if ( virt == NULL ) {
        return;
    }

    if ( !paging_map((uintptr_t) base, (uintptr_t) virt, PAGE_PRESENT | PAGE_RW) ) {
        return;
    }

    apic_base = base;

    // Enable bit 8 in the spurius interrupt vector to get interrupts
    outportb(0xF0, inportb(0xF0) | 0x100);

    kprintf("apic setup at %p.\n", apic_base);
}
