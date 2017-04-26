#include <stdbool.h>
#include <cpuid.h>
#include <kprint.h>

#ifndef bit_APIC
#define bit_APIC 0x00000200
#endif

bool apic_supported() {
    unsigned int _unused, edx;
    int success = __get_cpuid(0x01, &_unused, &_unused, &_unused, &edx);

    if ( success == 0 ) {
        kprintf("cpuid failed\n");
        return false;
    }

    return (edx & bit_APIC) != 0;
}
