#include <stdint.h>
#include <stdbool.h>

// Basic APIC commands
bool apic_supported();

// Local APIC commands
uintptr_t lapic_get_base();
void lapic_eoi();
void lapic_enable();
bool lapic_is_enabled();

// I/O APIC commands
void ioapic_register_writel(uint8_t offset, uint32_t val);
uint32_t ioapic_register_readl(uint8_t offset);
uint32_t ioapic_get_id();
uint32_t ioapic_get_version();
uint32_t ioapic_get_irqs();
void ioapic_setup(uintptr_t base);
