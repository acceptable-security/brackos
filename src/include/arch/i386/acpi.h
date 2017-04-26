#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} __attribute__((packed)) gas_t;

typedef struct {
   uint8_t  signature[4];
   uint32_t length;
   uint8_t  revision;
   uint8_t  checksum;
   uint8_t  oem[6];
   uint8_t  oem_table[8];
   uint32_t oem_revision;
   uint32_t creator_id;
   uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_t;

typedef struct {
    acpi_sdt_t table;
    uint32_t* pointers;
} __attribute__((packed)) acpi_rsdt_t;

typedef struct {
    acpi_sdt_t table;
    uint32_t firmware_ctrl;
    uint32_t dsdt;

    uint8_t  _reserved;

    uint8_t  preferred_powermanagement_profile;
    uint16_t sci_interrupt;
    uint32_t smi_cmd_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_req;
    uint8_t  pstate_ctrl;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_ctrl_block;
    uint32_t pm1b_ctrl_block;
    uint32_t pm2_ctrl_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_Block;
    uint32_t gpe1_Block;
    uint8_t  pm1_event_length;
    uint8_t  pm1_ctrl_length;
    uint8_t  pm2_ctrl_length;
    uint8_t  pm_timer_length;
    uint8_t  gpe0_Length;
    uint8_t  gpe1_Length;
    uint8_t  gpe1_Base;
    uint8_t  c_state_ctrl;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;
    uint8_t  month_alarm;
    uint8_t  century;

    uint16_t boot_arch_flags;

    uint8_t  _reserved2;
    uint32_t flags;

    gas_t reset_reg;
    uint8_t reset_val;

    uint8_t _reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;

    gas_t x_pm1a_event_block;
    gas_t x_pm1b_event_block;
    gas_t x_pm1a_ctrl_block;
    gas_t x_pm1b_ctrl_block;
    gas_t x_pm2_ctrl_block;
    gas_t x_pm_timer_block;
    gas_t x_gpe0_block;
    gas_t x_gpe1_block;
} __attribute__((packed)) acpi_fadt_t;

typedef struct {
    acpi_sdt_t table;
    uint32_t local_controller_address;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_lapic_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint8_t id;
    uint8_t reserved;
    uint32_t address;
    uint32_t global_system_interrupt_base;
} __attribute__((packed)) acpi_madt_ioapic_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__((packed)) acpi_madt_iso_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint16_t flags;
    uint32_t global_system_interrupt;
} __attribute__((packed)) acpi_madt_nmi_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint8_t processor_id;
    uint16_t flags;
    uint8_t lint;
} __attribute__((packed)) acpi_madt_lapic_nmi_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint16_t reserved;
    uint64_t lapic_address;
} __attribute__((packed)) acpi_madt_lapic_ao_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint8_t io_sapic_id;
    uint8_t reserved;
    uint32_t global_system_interrupt_base;
    uint64_t io_sapic_address;
} __attribute__((packed)) acpi_madt_io_sapic_t;

typedef struct {
    uint8_t type;
    uint8_t length;

    uint8_t processor_id;
    uint8_t local_sapic_id;
    uint8_t local_sapic_eid;
    uint8_t reserved[3];
    uint32_t flags;
    uint32_t processor_uid_value;
    uint8_t* processor_uid_string; // Should point to itself.
} __attribute__((packed)) acpi_madt_local_sapic_t;

typedef struct {
    uint8_t sig[8];
    uint8_t checksum;
    uint8_t oem[6];
    uint8_t revision;
    uint32_t rsdt;
} __attribute__((packed)) rsdp_desc_t;

typedef struct {
    rsdp_desc_t orig;
    uint32_t length;
    uint64_t xsdt;
    uint8_t checksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_ext_desc_t;

rsdp_desc_t* rsdp_locate();

bool acpi_validate(void* ptr, unsigned long len);
void acpi_parse_table(acpi_sdt_t* ptr);
bool acpi_parse_rsdt(acpi_rsdt_t* rsdt);
void acpi_parse_fadt(acpi_fadt_t* fadt);
bool acpi_init();
