#include <arch/i386/io.h>
#include <kernel/pci.h>
#include <kernel/pci_str.h>
#include <kprint.h>
#include <stdlib.h>

pci_devlist_t* devlist;

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

const char* pci_class_string(uint8_t class) {
    for ( int i = 0; i < sizeof(pci_class_codes1) / sizeof(pci_class_codes1[0]); i++ ) {
        if ( pci_class_codes1[i].code == (uint32_t) class ) {
            return pci_class_codes1[i].msg;
        }
    }

    return "Unknown";
}

const char* pci_subclass_string(uint8_t class, uint8_t subclass) {
    uint32_t search = (((uint32_t) class) << 8) | ((uint32_t) subclass);

    for ( int i = 0; i < sizeof(pci_class_codes2) / sizeof(pci_class_codes2[0]); i++ ) {
        if ( pci_class_codes2[i].code == search ) {
            return pci_class_codes2[i].msg;
        }
    }

    return "Unknown";
}

inline uint32_t pci_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (((uint32_t) bus << 16) | ((uint32_t) slot << 11) |
            ((uint32_t) func << 8) | ((uint32_t) offset & 0xfc) |
            (1 << 31));    
}

uint16_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = pci_address(bus, slot, func, offset);
    outportl(PCI_CONFIG_ADDRESS, address);
    uint32_t data = inportl(PCI_CONFIG_DATA);
    uint16_t out = (uint16_t) ((data >> ((offset & 2) << 3)) & 0xffff);
    return out;
}

void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data) {
    uint32_t address = pci_address(bus, slot, func, offset);
    outportl(PCI_CONFIG_ADDRESS, address);
    outportl(PCI_CONFIG_DATA, data);
}

pci_device_t* pci_device_new(uint16_t bus, uint16_t slot, uint16_t function) {
    uint16_t vendor = pci_config_read(bus, slot, function, 0);

    if ( vendor == 0xFFFF ) {
        return 0;
    }

    pci_device_t* dev_obj = (pci_device_t*) kmalloc(sizeof(pci_device_t));

    dev_obj->bus = bus;
    dev_obj->slot = slot;
    dev_obj->function = function;
    dev_obj->vendor = vendor;
    dev_obj->device = pci_config_read(bus, slot, function, 2);

    dev_obj->command = pci_config_read(bus, slot, function, 4);
    dev_obj->status = pci_config_read(bus, slot, function, 6);

    dev_obj->revision = (pci_config_read(bus, slot, function, 8) & 0xFF);
    dev_obj->prog_if = (pci_config_read(bus, slot, function, 8) >> 8) & 0xFF;
    dev_obj->subclass = (pci_config_read(bus, slot, function, 10) & 0xFF);
    dev_obj->class = (pci_config_read(bus, slot, function, 10) >> 8) & 0xFF;

    dev_obj->cacheline_size = (pci_config_read(bus, slot, function, 12) & 0xFF);
    dev_obj->latency_timer = (pci_config_read(bus, slot, function, 12) >> 8) & 0xFF;
    dev_obj->header_type = pci_config_read(bus, slot, function, 14) & 0x7F;
    dev_obj->multi_fn = (pci_config_read(bus, slot, function, 14) >> 7) & 1;
    dev_obj->bist = (pci_config_read(bus, slot, function, 14) >> 8) & 0xFF;

    switch ( dev_obj->header_type ) {
        case 0: ;
            // Normal

            pci_device_normal_t* extra = (pci_device_normal_t*) kmalloc(sizeof(pci_device_normal_t));

            extra->baseaddr_0 = (pci_config_read(bus, slot, function, 0x10)) | (pci_config_read(bus, slot, function, 0x12) << 16);
            extra->baseaddr_1 = (pci_config_read(bus, slot, function, 0x14)) | (pci_config_read(bus, slot, function, 0x16) << 16);
            extra->baseaddr_2 = (pci_config_read(bus, slot, function, 0x18)) | (pci_config_read(bus, slot, function, 0x18) << 16);
            extra->baseaddr_3 = (pci_config_read(bus, slot, function, 0x1C)) | (pci_config_read(bus, slot, function, 0x1E) << 16);
            extra->baseaddr_4 = (pci_config_read(bus, slot, function, 0x20)) | (pci_config_read(bus, slot, function, 0x22) << 16);
            extra->baseaddr_5 = (pci_config_read(bus, slot, function, 0x24)) | (pci_config_read(bus, slot, function, 0x26) << 16);
            extra->cardbus_cis = (pci_config_read(bus, slot, function, 0x28)) | (pci_config_read(bus, slot, function, 0x2A) << 16);

            extra->subsystem_vendor = pci_config_read(bus, slot, function, 0x2C);
            extra->subsystem = pci_config_read(bus, slot, function, 0x2E);

            extra->expansion_rom = (pci_config_read(bus, slot, function, 0x30) << 16) | pci_config_read(bus, slot, function, 0x32);

            extra->capability = pci_config_read(bus, slot, function, 0x34) & 0xFF;

            extra->interrupt_line = pci_config_read(bus, slot, function, 0x3C) & 0xFF;
            extra->interrupt_pin = pci_config_read(bus, slot, function, 0x3D) & 0xFF;
            extra->min_grant = pci_config_read(bus, slot, function, 0x3E) & 0xFF;
            extra->max_latency = pci_config_read(bus, slot, function, 0x3F) & 0xFF;

            dev_obj->extra = (void*) extra;

            break;

        case 1: ;
            // PCI-PCI bridge

            pci_device_pcipci_t* extra1 = (pci_device_pcipci_t*) kmalloc(sizeof(pci_device_pcipci_t));

            extra1->baseaddr_0 = (pci_config_read(bus, slot, function, 0x10)) | (pci_config_read(bus, slot, function, 0x12) << 16);
            extra1->baseaddr_1 = (pci_config_read(bus, slot, function, 0x14)) | (pci_config_read(bus, slot, function, 0x16) << 16);

            extra1->primary_bus = pci_config_read(bus, slot, function, 0x18) & 0xFF;
            extra1->secondary_bus = (pci_config_read(bus, slot, function, 0x18) >> 8) & 0xFF;
            extra1->subordinate_bus = pci_config_read(bus, slot, function, 0x1A) & 0xFF;
            extra1->secondary_latency = (pci_config_read(bus, slot, function, 0x1A) >> 8) & 0xFF;

            extra1->io_base = pci_config_read(bus, slot, function, 0x1C) & 0xFF;
            extra1->io_limit = (pci_config_read(bus, slot, function, 0x1C) >> 8) & 0xFF;
            extra1->secondary_status = pci_config_read(bus, slot, function, 0x1E);

            extra1->prefetch_mem_base = pci_config_read(bus, slot, function, 0x24);
            extra1->prefetch_mem_limit = pci_config_read(bus, slot, function, 0x26);

            extra1->prefetch_base = (pci_config_read(bus, slot, function, 0x28)) | (pci_config_read(bus, slot, function, 0x2A) << 16);
            extra1->prefetch_limit = (pci_config_read(bus, slot, function, 0x2C)) | (pci_config_read(bus, slot, function, 0x2E) << 16);

            extra1->io_upperbase = pci_config_read(bus, slot, function, 0x30);
            extra1->io_upperlimit = pci_config_read(bus, slot, function, 0x32);

            extra1->capability = pci_config_read(bus, slot, function, 0x34) & 0xFF;

            extra1->expansion_rom = (pci_config_read(bus, slot, function, 0x38)) | (pci_config_read(bus, slot, function, 0x3A) << 16);

            extra1->interrupt_line = pci_config_read(bus, slot, function, 0x3C) & 0xFF;
            extra1->interrupt_pin = (pci_config_read(bus, slot, function, 0x3C) >> 8) & 0xFF;
            extra1->bridge_control = pci_config_read(bus, slot, function, 0x3E);

            dev_obj->extra = (void*) extra1;

            break;

        case 2: ;
            // PCI-to-CardBus
            break;

        default: ;
            kprintf("pci: Unable to deal with device 0x%X\n", dev_obj->header_type);

            kfree(dev_obj);
            return 0;
    }

    return dev_obj;
}

uint32_t pci_device_base_addr(pci_device_t* pci, unsigned int n) {
    switch ( pci->header_type ) {
        case 0: {
            pci_device_normal_t* header = (pci_device_normal_t*) pci->extra;

            switch ( n ) {
                case 0: return header->baseaddr_0;
                case 1: return header->baseaddr_1;
                case 2: return header->baseaddr_2;
                case 3: return header->baseaddr_3;
                case 4: return header->baseaddr_4;
                case 5: return header->baseaddr_5;
                default: return 0;
            }

            break;
        }
        case 1: {
            pci_device_pcipci_t* header = (pci_device_pcipci_t*) pci->extra;

            switch ( n ) {
                case 0: return header->baseaddr_0;
                case 1: return header->baseaddr_1;
                default: return 0;
            }

            break;
        }

        default: {
            return 0;
        }
    }
}

pci_cmd_reg_t pci_device_read_cmd(pci_device_t* device) {
    return (pci_cmd_reg_t) pci_config_read(device->bus,
                                           device->slot,
                                           device->function, 4);
}

void pci_device_write_cmd(pci_device_t* device, pci_cmd_reg_t val) {
    pci_config_write(device->bus, 
                     device->slot,
                     device->function,
                     4,
                     val.data);
}

void pci_probe() {
    uint32_t bus, slot, function;

    for ( bus = 0; bus < 256; bus++ ) {
        for ( slot = 0; slot < 32; slot++ ) {
            for ( function = 0; function < 8; function++ ) {
                pci_device_t* dev_obj = pci_device_new(bus, slot, function);

                if ( dev_obj == NULL ) {
                    continue;
                }

                kprintf("pci: [%x:%x] %s / %s Found (status %d / prog_if %x).\n",
                    dev_obj->vendor,
                    dev_obj->device,
                    pci_class_string(dev_obj->class),
                    pci_subclass_string(dev_obj->class, dev_obj->subclass),
                    dev_obj->status,
                    dev_obj->prog_if);

                pci_devlist_add(devlist, dev_obj);
            }
        }
    }
}

pci_devlist_t* pci_devlist_new(int max) {
    pci_devlist_t* list = (pci_devlist_t*) kmalloc(sizeof(pci_devlist_t));

    list->devices = (pci_device_t**) kmalloc(sizeof(pci_device_t*) * max);
    list->size = 0;
    list->max = max;

    return list;
}

void pci_devlist_add(pci_devlist_t* list, pci_device_t* device) {
    if ( list->size + 1 >= list->max ) {
        kprintf("pci: ERROR: Max PCI devices hit!");
        return;
    }

    list->devices[list->size] = device;
    list->size++;

    device->dev_id = list->size - 1;
}

pci_device_t* pci_get(uint16_t vendor, uint16_t device) {
    for ( int i = 0; i < devlist->size; i++ ) {
        pci_device_t* dev = devlist->devices[i];

        if ( dev->vendor == vendor && dev->device == device ) {
            return dev;
        }
    }

    return NULL;
}

void pci_init() {
    devlist = pci_devlist_new(PCI_MAXDEVS);

    pci_probe();
}