#include <arch/i386/irq.h>
#include <devices/pci.h>
#include <stdint.h>

// RTL8139 registers
#define RTL8139_REG_MAC 0x00
#define RTL8139_REG_MAR 0x00
#define RTL8139_REG_TSR 0x08
#define RTL8139_REG_TSD 0x10
#define RTL8139_REG_TSAD 0x20
#define RTL8139_REG_TNPDS 0x20
#define RTL8139_REG_THPDS 0x28
#define RTL8139_REG_RBSTART 0x30
#define RTL8139_REG_ERBCR 0x34
#define RTL8139_REG_ERSR 0x36
#define RTL8139_REG_CMD 0x37
#define RTL8139_REG_IMR 0x3C
#define RTL8139_REG_ISR 0x3E
#define RTL8139_REG_TCR 0x40
#define RTL8139_REG_RCR 0x44
#define RTL8139_REG_93C46_CMD 0x50
#define RTL8139_REG_CONF0 0x51
#define RTL8139_REG_CONF1 0x52

// RTL8139 Commands
#define RTL8139_CMD_START_RXTX 0x10
#define RTL8139_CMD_RESET 0x10

// Size of the receive buffer (3 pages w/ 4K pages)
#define RTL8139_BUFF_SIZE 0x3000
#define RTL8139_MAX_PACKET 1792

typedef struct {
	// Device information
	uint32_t io_addr;
	uint32_t mem_addr;
	uint8_t irq;
	pci_device_t* pci_dev;

	// Buffer information
	void* recv_buff_virt;
	void* recv_buff_phys;

	void* tx_buff_virt[4];
	void* tx_buff_phys[4];
	uint8_t curr_buff;

	// Ethernet Information
	uint8_t mac[6];
} rtl8139_dev_t;

// Command Register
typedef union {
	uint8_t data;
	struct {
		uint8_t bufe  : 1; // Buffer Empty
		uint8_t resv1 : 1; // Reserved
		uint8_t te    : 1; // Transmitter Enable
		uint8_t re    : 1; // Receiver Enable
		uint8_t rst   : 1; // Reset
		uint8_t resv2 : 3; // Reserved
	};
} rtl8139_cmd_t;

// Interrupt Status/Mask register
union rtl8139_int_reg {
	uint16_t data;
	struct {
		uint16_t rok     : 1; // Rx OK Interrupt
		uint16_t rer     : 1; // Rx Error Interrupt
		uint16_t tok     : 1; // Tx (OK) Interrupt
		uint16_t ter     : 1; // Tx Error Interrupt
		uint16_t rbo     : 1; // Rx Buffer Overflow/Rx Descriptor Unavailable Interrupt
		uint16_t pun     : 1; // Packet UNderrun Link Change Interrupt;
		uint16_t fovw    : 1; // Rx FIFO Overflow Interrupt
		uint16_t tdu     : 1; // Tx Descriptor Unavailable Interrupt
		uint16_t swint   : 1; // Software Interrupt
		uint16_t resrv   : 4; // Reserved 
		uint16_t lenchg  : 1; // Cable Length Change Interrupt
		uint16_t timeout : 1; // Time Out Interrupt
		uint16_t serr    : 1; // System Error Interrupt
	};
};

typedef union rtl8139_int_reg rtl8139_imr_t;
typedef union rtl8139_int_reg rtl8139_isr_t;

// Transmit Configuration Register
typedef enum {
	RTL8139_VER_RTL8139   = 0b110000,
	RTL8139_VER_RTL8139A  = 0b111000,
	RTL8139_VER_RTL8139AG = 0b111001,
	RTL8139_VER_RTL8139B  = 0b111100,
	RTL8139_VER_RTL8130   = 0b111110,
	RTL8139_VER_RTL8139C  = 0b111010,
	RTL8139_VER_RTL8139CP = 0b111011
} rtl8139_ver_t;

typedef union {
	uint32_t data;
	struct {
		uint32_t clrabt    : 1;  // Clear Abort
		uint32_t reserved  : 3;  // Reserved
		uint32_t txrr      : 4;  // Tx Retry Count
		uint32_t mxdma2    : 3;  // Max DMA Burst Size per Tx DMA Burst
		uint32_t reserved2 : 5;  // Reserved
		uint32_t crc       : 1;  // Append CRC
		uint32_t loopback  : 2;  // Loopback Test
		uint32_t ifg2      : 1;  // Interface Gap 2
		uint32_t reserved3 : 3;  // Reserved
		uint32_t revG      : 1;  // RTL8139A rev.G ID = 1. For others, this bit 0.
		uint32_t ifg       : 2;  // Interframe Gap Time
		uint32_t hwverid   : 5; // Hardware Version ID
		uint32_t reserved4 : 1;  // Reserved
	};
} rtl8139_tcr_t;

// Receiver Configuration Register
typedef union {
	uint32_t data;
	struct {
		uint32_t aap       : 1; // Accept Physical Address Packets
		uint32_t apm       : 1; // Accept Physical Match Packets
		uint32_t am        : 1; // Accept Multicast Packets
		uint32_t ab        : 1; // Accept Broadcast Packets
		uint32_t ar        : 1; // Accept Runt Packets
		uint32_t aer       : 1; // Accept Error Packets
		uint32_t sel9356   : 1; // EEPROM Select
		uint32_t wrap      : 1; // Enable/disable overflowing/wrapping
		uint32_t mxdma     : 3; // Max DMA Burst Size per Rx DMA Burst
		uint32_t rblen     : 2; // Rx Buffer Length
		uint32_t rxfth	   : 3; // Rx FIFO Threshold
		uint32_t rer8      : 1; // Receive Error Packets Larger than 8 Bytes
		uint32_t mulerint  : 1; // Multiple Early Interrupt Select
		uint32_t reserve   : 6; // Reserved
		uint32_t erth      : 4; // Early Rx Threshold Bits
		uint32_t reserved2 : 4; // Reserved
	};
} rtl8139_rcr_t;

// Transaction Status Description
typedef union {
	uint32_t data;
	struct {
		uint32_t size     : 13; // Descriptor Size
		uint32_t own      : 1; // OWN
		uint32_t tun      : 1; // Transaction FIFO Underrun
		uint32_t tok      : 1; // Transmit OK
		uint32_t ertxth   : 6; // Early Tx Threshold
		uint32_t reserved : 2; // reserved
		uint32_t ncc      : 4; // Number of Collision Count
		uint32_t cdh      : 1; // CD Heart Beat
		uint32_t owc      : 1; // Out of Window Collision
		uint32_t tabt     : 1; // Transmit Abort
		uint32_t crs      : 1; // Carrier Sense Lost
	};
} rtl8139_tsd_t;

// Packet Header
typedef union {
	uint32_t data;
	struct {
		uint32_t rok  : 1; // Receive OK
		uint32_t fae  : 1; // Frame Alignment Error
		uint32_t crc  : 2; // CRC Error
		uint32_t lng  : 1; // Long Packet
		uint32_t runt : 1; // Runt Packet
		uint32_t ise  : 1; // Invalid Symbol Error
		uint32_t resv : 6; // Reserved
		uint32_t bar  : 1; // Broadcast Adress Received
		uint32_t pam  : 1; // Physical Address Matched
		uint32_t mar  : 1; // Multicast Address Received
		uint32_t len  : 16; // Length of the packet
	};
} rtl8139_packet_t;

void rtl8139_interrupt(irq_regs_t* frame);

void rtl8139_poweron(rtl8139_dev_t* dev);
void rtl8139_softreset(rtl8139_dev_t* dev);
void rtl8139_load_buffer(rtl8139_dev_t* dev);
void rtl8139_set_int(rtl8139_dev_t* dev);
void rtl8139_read_mac(rtl8139_dev_t* dev);
void rtl8139_set_rcr(rtl8139_dev_t* dev);
void rtl8139_set_tcr(rtl8139_dev_t* dev);
void rtl8139_enable_rxtx(rtl8139_dev_t* dev);
void rtl8139_reg_irq(rtl8139_dev_t* dev);
rtl8139_tsd_t rtl8139_get_tsd(rtl8139_dev_t* dev, int desc);
rtl8139_isr_t rtl8139_get_isr(rtl8139_dev_t* dev, int ack);

void rtl8139_debug_tsd(int i, rtl8139_tsd_t tsd);

void rtl8139_init();