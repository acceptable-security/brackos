#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>

#ifndef bit_MSR
#define bit_MSR 0x00000020
#endif

#ifndef bit_APIC
#define bit_APIC 0x00000200
#endif

#define IA32_APIC_BASE_MSR        0x1B
#define IA32_APIC_BASE_MSR_BSP    0x100
#define IA32_APIC_BASE_MSR_X2APIC 0x400
#define IA32_APIC_BASE_MSR_ENABLE 0x800

bool cpu_has_msr() {
    uint32_t _unused, eax, edx;
    __get_cpuid(1, &eax, &edx, &_unused, &_unused);
    return edx & bit_MSR;
}

void cpu_get_msr(uint32_t msr, uint32_t *low, uint32_t *high) {
    __asm__ volatile("rdmsr" : "=a" (*low), "=d" (*high) : "c" (msr));
}

void cpu_set_msr(uint32_t msr, uint32_t low, uint32_t high) {
   __asm__ volatile("wrmsr" : : "a" (low), "d" (high), "c" (msr));
}
