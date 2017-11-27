#include <arch/i386/msr.h>
#include <arch/i386/io.h>
#include <arch/i386/paging.h>
#include <arch/i386/irq.h>
#include <mem/vasa.h>
#include <stdlib.h>
#include <stdbool.h>
#include <kprint.h>

extern void irq_empty_stub();

#define APIC_REG_ID                  0x0020
#define APIC_REG_VERSION             0x0030
#define APIC_REG_TASK_PRIORITY       0x0080
#define APIC_REG_END_OF_INTERRUPT    0x00B0
#define APIC_REG_LOGICAL_DESTINATION 0x00D0
#define APIC_REG_DESTINATION_FORMAT  0x00E0
#define APIC_REG_SPURIOUS_INTERRUPT  0x00F0
#define APIC_REG_INSERVICE_ROUTINE   0x0100
#define APIC_REG_ERROR_STATUS        0x0280
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
#define APIC_ICR_CMD_INIT            0x0500
#define APIC_ICR_CMD_STARTUP         0x0600
#define APIC_ICR_FIXED               0x0000
#define APIC_ICR_LOWEST              0x0100
#define APIC_ICR_SMI                 0x0200
#define APIC_ICR_NMI                 0x0400
#define APIC_ICR_PHYSICAL            0x0000
#define APIC_ICR_LOGICAL             0x0800
#define APIC_ICR_IDLE                0x0000
#define APIC_ICR_SEND_PENDING        0x1000
#define APIC_ICR_DEASSERT            0x0000
#define APIC_ICR_ASSERT              0x4000
#define APIC_ICR_EDGE                0x0000
#define APIC_ICR_LEVEL               0x8000

uintptr_t lapic_base = 0;

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

bool lapic_is_x2apic() {
    uint32_t eax, edx;
    cpu_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);

    const uint32_t flags = IA32_APIC_BASE_MSR_X2APIC;

    return (eax & flags) == flags;
}

// Set the physical address of the APIC
void lapic_set_base(uintptr_t apic) {
    uint32_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
    cpu_set_msr(IA32_APIC_BASE_MSR, eax, 0);
}

// Return the physical address of the APIC
uintptr_t lapic_get_base() {
    uint32_t eax, edx;
    cpu_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);
    return (eax & 0xfffff000);
}

// Read to an apic register
uint32_t lapic_register_readl(uint32_t reg) {
    return *(volatile uint32_t*)(lapic_base + reg);
}

// Write to an apic register
void lapic_register_writel(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(lapic_base + reg) = value;
    (void) lapic_register_readl(APIC_REG_ID);
}

uint32_t lapic_get_id() {
    return lapic_register_readl(APIC_REG_ID) >> 24;
}

// Send a init command to another APIC
void lapic_send_init(uint32_t apic) {
    lapic_register_writel(APIC_REG_INTERRUPT_CMD_HIGH, apic << 24);

    lapic_register_writel(APIC_REG_INTERRUPT_CMD_LOW,
        APIC_ICR_CMD_INIT |
        APIC_ICR_PHYSICAL |
        APIC_ICR_EDGE     |
        APIC_ICR_ASSERT);

    while ( lapic_register_readl(APIC_REG_INTERRUPT_CMD_LOW) & APIC_ICR_SEND_PENDING );
}

// Send a init command to another APIC
void lapic_send_startup(uint32_t apic, uint32_t vec) {
    lapic_register_writel(APIC_REG_INTERRUPT_CMD_HIGH, apic << 24);

    lapic_register_writel(APIC_REG_INTERRUPT_CMD_LOW,
        vec                  |
        APIC_ICR_CMD_STARTUP |
        APIC_ICR_PHYSICAL    |
        APIC_ICR_EDGE        |
        APIC_ICR_ASSERT);

    while ( lapic_register_readl(APIC_REG_INTERRUPT_CMD_LOW) & APIC_ICR_SEND_PENDING );
}

// Send the End of Interrupt to the APIC
void lapic_eoi() {
    lapic_register_writel(APIC_REG_END_OF_INTERRUPT, 0);
}

// Determine if the APIC is enabled
bool lapic_is_enabled() {
    uint32_t eax, edx;
    cpu_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);
    return (eax & IA32_APIC_BASE_MSR_ENABLE) > 0 && lapic_base != 0;
}

// Enable the spurious interrupt vector
void lapic_enable_spurious_interrupt(uint32_t intr) {
    lapic_register_writel(APIC_REG_SPURIOUS_INTERRUPT, intr | 0x100);
}

// Get the current in service routine for the current lapic
int lapic_inservice_routine() {
    for ( int i = 0; i < 8; i++ ) {
        uint32_t isr = lapic_register_readl(APIC_REG_INSERVICE_ROUTINE + (0x10 * i));

        if ( isr > 0 ) {
            return ((i * 32) + __builtin_ctz(isr)) - 0x20;
        }
    }

    return -1;
}

// Clear error status register
void lapic_clear_error() {
    kprintf("error status: %x\n", lapic_register_readl(APIC_REG_ERROR_STATUS));
    lapic_register_writel(APIC_REG_ERROR_STATUS, 0);
    lapic_register_writel(APIC_REG_ERROR_STATUS, 0);
}

// Enable the APIc
void lapic_enable() {
    // Hardware enable APIC
    uintptr_t base = lapic_get_base();

    kprintf("lapic: base is %x\n", base);
    kprintf("lapic: x2apic status %d\n", lapic_is_x2apic());

    uintptr_t virt = (uintptr_t) vasa_alloc(MEM_PCI, 4096, 0);

    if ( virt == 0 ) {
        kprintf("lapic: failed to allocate a lapic virt addr\n");
        return;
    }

    if ( !paging_map(base, virt, PAGE_PRESENT | PAGE_RW) ) {
        kprintf("lapic: failed to map the lapic in\n");
        return;
    }

    lapic_base = virt;

    lapic_register_writel(APIC_REG_TASK_PRIORITY, 0);                // Enable all interrupts
    lapic_register_writel(APIC_REG_DESTINATION_FORMAT, 0xFFFFFFFF);  // Flat mode
    lapic_register_writel(APIC_REG_LOGICAL_DESTINATION, 1 << 24); // Logical CPU 1

    // Create spurious interrupt in IDT and enable it
    idt_set_gate(0xFF, (uintptr_t) irq_empty_stub, 0x08, 0x8E);
    lapic_enable_spurious_interrupt(0xFF);


    kprintf("lapic: setup at %p.\n", lapic_base);
    kprintf("lapic: id %d\n", (lapic_register_readl(APIC_REG_ID) >> 24));
    kprintf("lapic: version is %x\n", lapic_register_readl(APIC_REG_VERSION));
}
