#include <stdint.h>
#include <cpuid.h>

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
