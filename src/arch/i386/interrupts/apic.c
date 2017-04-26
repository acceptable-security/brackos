#include <arch/i386/msr.h>
#include <arch/i386/io.h>
#include <stdbool.h>
#include <kprint.h>

#ifndef bit_APIC
#define bit_APIC 0x00000200
#endif

#define IA32_APIC_BASE_MSR        0x1B
#define IA32_APIC_BASE_MSR_BSP    0x100
#define IA32_APIC_BASE_MSR_ENABLE 0x800

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

void apic_enable() {
    // Hardware enable APIC
    apic_set_base(apic_get_base());

    // Enable bit 8 in the spurius interrupt vector to get interrupts
    outportb(0xF0, inportb(0xF0) | 0x100);

    kprintf("APIC initialized.\n");
}
