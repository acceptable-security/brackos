#include <stdint.h>
#include <stdbool.h>

// Basic APIC commands
bool apic_supported();

// Local APIC commands
uintptr_t lapic_get_base();
void lapic_eoi();
void lapic_enable();
bool lapic_is_enabled();
int lapic_inservice_routine();
uint32_t lapic_get_id();
void lapic_send_init(uint32_t apic);
void lapic_send_startup(uint32_t apic, uint32_t vec);

// I/O APIC commands
void ioapic_register_writel(uint8_t offset, uint32_t val);
uint32_t ioapic_register_readl(uint8_t offset);
uint32_t ioapic_get_id();
uint32_t ioapic_get_version();
uint32_t ioapic_get_irqs();
void ioapic_setup(uintptr_t base);
void ioapic_enable_irq(uint8_t irq, uint8_t vector);

// SMP commands
void smp_init();
