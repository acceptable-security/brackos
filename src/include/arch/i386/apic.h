#include <stdint.h>
#include <stdbool.h>

bool apic_supported();
uintptr_t apic_get_base();
void apic_enable();
