#include <stdlib.h>

typedef struct {
    uint32_t code;
    const char* msg;
} pci_msg_t;

#define PCI_MSG(CODE, MSG) { .code = (CODE), .msg = (MSG) }

pci_msg_t pci_class_codes1[] = {
    PCI_MSG(0x00, "Unclassified"),
    PCI_MSG(0x01, "Mass Storage Controller"),
    PCI_MSG(0x02, "Network Controller"),
    PCI_MSG(0x03, "Display Controller"),
    PCI_MSG(0x04, "Multimedia Controller"),
    PCI_MSG(0x05, "Memory Controller"),
    PCI_MSG(0x06, "Bridge Device"),
    PCI_MSG(0x07, "Simple Communication Controller"),
    PCI_MSG(0x08, "Base System Peripheral"),
    PCI_MSG(0x09, "Input Device Controller"),
    PCI_MSG(0x0A, "Docking Station"),
    PCI_MSG(0x0B, "Processor"),
    PCI_MSG(0x0C, "Serial Bus Controller"),
    PCI_MSG(0x0D, "Wireless Controller"),
    PCI_MSG(0x0E, "Intelligent Controller"),
    PCI_MSG(0x0F, "Satellite Communication Controller"),
    PCI_MSG(0x10, "Encryption Controller"),
    PCI_MSG(0x11, "Signal Processing Controller"),
    PCI_MSG(0x12, "Processing Accelerator"),
    PCI_MSG(0x13, "Non-Essential Instrumentation"),
    PCI_MSG(0x40, "Co-Processor")
};

pci_msg_t pci_class_codes2[] = {
    // Unclassified Subclasses
    PCI_MSG(0x0000, "Non-VGA-Compatible Device"),
    PCI_MSG(0x0001, "VGA-Compatible Device"),

    // Mass Stoarge Subclasses
    PCI_MSG(0x0100, "SCSI Bus Controller"),
    PCI_MSG(0x0101, "IDE Controller"),
    PCI_MSG(0x0102, "Floppy Disk Controller"),
    PCI_MSG(0x0103, "IPI Bus Controller"),
    PCI_MSG(0x0104, "RAID Controller"),
    PCI_MSG(0x0105, "ATA Controller"),
    PCI_MSG(0x0106, "Serial ATA"),
    PCI_MSG(0x0107, "Serial Attached SCSI"),
    PCI_MSG(0x0108, "Non-Volatile Memory Controller"),
    PCI_MSG(0x0180, "Other"),

    // Network Controller subclasses
    PCI_MSG(0x0200, "Ethernet Controller"),
    PCI_MSG(0x0201, "Token Ring Controller"),
    PCI_MSG(0x0202, "FDDI Controller"),
    PCI_MSG(0x0203, "ATM Controller"),
    PCI_MSG(0x0204, "ISDN Controller"),
    PCI_MSG(0x0205, "WorldFip Controller"),
    PCI_MSG(0x0206, "PICMG 2.14 Multi Computing"),
    PCI_MSG(0x0207, "Infiniband Controller"),
    PCI_MSG(0x0208, "Fabric Controller"),
    PCI_MSG(0x0280, "Other"),
    PCI_MSG(0x0300, "VGA Campatible Controller"),
    PCI_MSG(0x0301, "XGA Controller"),
    PCI_MSG(0x0302, "3D Controller"),
    PCI_MSG(0x0380, "Other"),

    // Multimedia Controller
    PCI_MSG(0x0400, "Multimedia Video Controller"),
    PCI_MSG(0x0401, "Multimedia Audio Controller"),
    PCI_MSG(0x0402, "Computer Telephony Device"),
    PCI_MSG(0x0403, "Audio Device"),
    PCI_MSG(0x0480, "Other"),

    // Memory Controller
    PCI_MSG(0x0500, "RAM Controller"),
    PCI_MSG(0x0501, "Flash Controller"),
    PCI_MSG(0x0580, "Other"),

    // Bridge Device
    PCI_MSG(0x0600, "Host Bridge"),
    PCI_MSG(0x0601, "ISA Bridge"),
    PCI_MSG(0x0602, "EISA Bridge"),
    PCI_MSG(0x0603, "MCA Bridge"),
    PCI_MSG(0x0604, "PCI-to-PCI bridge"),
    PCI_MSG(0x0605, "PCMCIA Bridge"),
    PCI_MSG(0x0606, "NuBus Bridge"),
    PCI_MSG(0x0607, "CardBus Bridge"),
    PCI_MSG(0x0608, "RACEway Bridge"),
    PCI_MSG(0x0609, "PCI-to-PCI Bridge"),
    PCI_MSG(0x060A, "InfiniBand-to-PCI Host Bridge"),
    PCI_MSG(0x0680, "Other"),

    // Simple Communication Controller
    PCI_MSG(0x0700, "Serial Controller"),
    PCI_MSG(0x0701, "Parallel Controller"),
    PCI_MSG(0x0702, "Multiport Serial Controller"),
    PCI_MSG(0x0703, "Modem"),
    PCI_MSG(0x0704, "IEEE 488.1/2 (GPIB) Controller"),
    PCI_MSG(0x0705, "Smart Card"),
    PCI_MSG(0x0780, "Other"),

    // Base System Peripheral
    PCI_MSG(0x0800, "PIC"),
    PCI_MSG(0x0801, "DMA Controller"),
    PCI_MSG(0x0802, "Timer"),
    PCI_MSG(0x0803, "RTC Controller"),
    PCI_MSG(0x0804, "PCI Hot-Plug Controller"),
    PCI_MSG(0x0805, "SD Host Controller"),
    PCI_MSG(0x0806, "IOMMU"),
    PCI_MSG(0x0880, "Other"),

    // Input Device Controller
    PCI_MSG(0x0900, "Keyboard Controller"),
    PCI_MSG(0x0901, "Digitizer Pen"),
    PCI_MSG(0x0902, "Mouse Controller"),
    PCI_MSG(0x0903, "Scanner Controller"),
    PCI_MSG(0x0904, "Gameport Controller"),
    PCI_MSG(0x0980, "Other"),

    // Docking Station
    PCI_MSG(0x0A00, "Docking Station"),
    PCI_MSG(0x0A80, "Generic"),

    // Processor
    PCI_MSG(0x0B00, "386"),
    PCI_MSG(0x0B01, "486"),
    PCI_MSG(0x0B02, "Pentium"),
    PCI_MSG(0x0B10, "Alpha"),
    PCI_MSG(0x0B20, "PowerPC"),
    PCI_MSG(0x0B30, "MIPS"),
    PCI_MSG(0x0B40, "Co-Processor"),

    // Serial Bus Controller
    PCI_MSG(0x0C00, "FireWire Controller"),
    PCI_MSG(0x0C01, "ACCESS Bus"),
    PCI_MSG(0x0C02, "SSA"),
    PCI_MSG(0x0C03, "USB Controller"),
    PCI_MSG(0x0C04, "Fibre Channel"),
    PCI_MSG(0x0C05, "SMBus"),
    PCI_MSG(0x0C06, "InfiniBand"),
    PCI_MSG(0x0C07, "IPMI Interface"),
    PCI_MSG(0x0C08, "SERCOS Interface"),
    PCI_MSG(0x0C09, "CANbus"),

    // Wireless Controller
    PCI_MSG(0x0D00, "iRDA Compatible Controller"),
    PCI_MSG(0x0D01, "Consumer IR Controller"),
    PCI_MSG(0x0D11, "Bluetooth Controller"),
    PCI_MSG(0x0D12, "Broadband Controller"),
    PCI_MSG(0x0D20, "Ethernet Controller (802.1a)"),
    PCI_MSG(0x0D21, "Ethernet Controller (802.1b)"),
    PCI_MSG(0x0D80, "Other"),

    // Intelligent Controller
    PCI_MSG(0x0E00, "I20"),

    // Encryption Controller
    PCI_MSG(0x0F01, "Satellite TV Controller"),
    PCI_MSG(0x0F02, "Satellite Audio Controller"),
    PCI_MSG(0x0F03, "Satellite Voice Controller"),
    PCI_MSG(0x0F04, "Satellite Data Controller"),

    // Signal Processing Contorller
    PCI_MSG(0x1100, "DPIO Modules"),
    PCI_MSG(0x1101, "Performance Counters"),
    PCI_MSG(0x1110, "Communication Synchronizer"),
    PCI_MSG(0x1120, "Signal Processing Management"),
    PCI_MSG(0x1180, "Other")
};