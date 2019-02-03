#include <arch/i386/apic.h>
#include <arch/i386/io.h>
#include <arch/i386/irq.h>
#include <arch/i386/paging.h>
#include <kernel/pci.h>
#include <drivers/rtl8139.h>
#include <kernel/config.h>
#include <kprint.h>
#include <mem/mmap.h>
#include <stdlib.h>
#include <string.h>

#ifdef BRACKOS_CONF_ACPI
#include <arch/i386/acpi.h>
#endif

#define RTL8139_PCI_VENDOR 0x10EC
#define RTL8139_PCI_DEVICE 0x8139

static rtl8139_dev_t* g_dev;

// Interrupt handler for the RTL8139
void rtl8139_interrupt(irq_regs_t* frame) {
	// Get ISR and ack
	rtl8139_isr_t isr = rtl8139_get_isr(g_dev, 1);

	kprintf("rtl8139: isr=%x\n", isr.data);

	if ( isr.rok == 1 ) {
		kprintf("rtl8139: rok and roll\n");

		rtl8139_packet_t header = *((rtl8139_packet_t*) g_dev->recv_buff_virt);

		// Data is formatted in the buffer as
		// [ 4 HEAD ][ N DATA ][ 4 CRC ]
		// buff is at the beginning of len, and len = N + 4
		// thus len will bring you to start of CRC.
		uint32_t crc = *(uint32_t*) (((uintptr_t) g_dev->recv_buff_virt) + header.len);
		char* data = (char*) (((uintptr_t) g_dev->recv_buff_virt) + 4);

		// Print some basic header information for now
		// Do CRC checks/ring buff later.
		kprintf("rtl8139: found %d bytes\n", header.len);
		kprintf("rtl8139: crc %x\n", crc);

		kprintf("rtl8139: header flags: ");
		if ( header.rok )  kprintf("rok ");
		if ( header.fae )  kprintf("fae ");
		if ( header.crc )  kprintf("crc ");
		if ( header.lng )  kprintf("lng ");
		if ( header.runt ) kprintf("runt ");
		if ( header.ise )  kprintf("ise ");
		if ( header.bar )  kprintf("bar ");
		if ( header.pam )  kprintf("pam ");
		if ( header.mar )  kprintf("mar ");
		kprintf("\n");

		// Read data into net buffer
		net_buf_read(g_dev->net_buf, data, header.len - 4);
	}
	
	if ( isr.tok == 1 ) {
		for ( int i = 0; i < 4; i++ ) {
			rtl8139_tsd_t tsd = rtl8139_get_tsd(g_dev, i);

			if ( tsd.tok == 1 ) {
				kprintf("rtl8139: packet sent from descriptor %d\n", i);
			}
		}
	}
}

// Send a packet
void rtl8139_send_packet(rtl8139_dev_t* dev, char* data, uint32_t len) {
	if ( len > RTL8139_MAX_PACKET ) {
		kprintf("rtl8139: cant send packet of %d len max is %d\n", len, RTL8139_MAX_PACKET);
		return;
	}

	kprintf("rtl8139: sending %d bytes\n", len);

	// Acquire a buffer
	uint8_t desc = dev->curr_buff;
	void* buf = dev->tx_buff_virt[desc];
	dev->curr_buff = (desc + 1) % 4;

	// Check TSD and wait for tok == 1
	uint32_t tsd_port = dev->io_addr + RTL8139_REG_TSD + (desc * 4);
	rtl8139_tsd_t tsd;

	// Wait for OWN == 1
	do {
		tsd = (rtl8139_tsd_t) inportl(tsd_port);
		io_wait();
	} while (tsd.own != 1);

	// Move data
	memcpy(buf, data, len);

	// Send out data
	tsd.data = 0;
	tsd.size = len;

	outportl(tsd_port, tsd.data);
	kprintf("rtl8139: data sent\n");

	// Wait for OWN == 1
	do {
		tsd = (rtl8139_tsd_t) inportl(tsd_port);
		io_wait();
	} while (tsd.own != 1);

	kprintf("rtl8139: dma complete\n");

	// Wait for TOK == 1
	do {
		tsd = (rtl8139_tsd_t) inportl(tsd_port);
		io_wait();
	} while (tsd.tok != 1);

	kprintf("rtl8139: send complete\n");
}

static int rtl8139_alloc_buffer(void** virt, void** phys) {
	// Allocate the memory (returns a virtual address)
	*virt = memmap(NULL, RTL8139_BUFF_SIZE, MMAP_RW | MMAP_URGENT);

	if ( *virt == NULL ) {
		return 0;
	}

	// Get the physical address
	*phys = (void*) paging_find_physical((uintptr_t) *virt);
	return 1;
}

// Initialize the receiver and sending buffers
void rtl8139_init_buffer(rtl8139_dev_t* dev) {
	if ( !rtl8139_alloc_buffer(&dev->recv_buff_virt,
							   &dev->recv_buff_phys) ) {
		kprintf("rtl8139: failed to get recv buffer");
		return;
	}

	kprintf("rtl8139: recv buffer at %p (mapped at %p)\n", dev->recv_buff_phys, dev->recv_buff_virt);

	for ( int i = 0; i < 4; i++ ) {
		if ( !rtl8139_alloc_buffer(&dev->tx_buff_virt[i],
								   &dev->tx_buff_phys[i]) ) {
			kprintf("rtl8139: failed to get recv buffer");
			return;
		}

		kprintf("rtl8139: tx buffer at %p (mapped at %p)\n", dev->tx_buff_phys[i], dev->tx_buff_virt[i]);
	}

	dev->net_buf = net_buf_init(RTL8139_RECV_BUFF);

	if ( !dev->net_buf ) {
		kprintf("rtl8139: failed to init netbuf\n");
		return;
	}

	kprintf("rtl8139: net buf of %d bytes allocated\n", RTL8139_RECV_BUFF);
}

// Run the startup sequence for the RTL8139
void rtl8139_startup(rtl8139_dev_t* dev) {
	kprintf("rtl8139: performing startup\n");
	rtl8139_poweron(dev);
	rtl8139_softreset(dev);
	rtl8139_enable_rxtx(dev);
	rtl8139_set_rcr(dev);
	rtl8139_set_tcr(dev);
	rtl8139_init_buffer(dev);
	rtl8139_load_buffer(dev);
	rtl8139_read_mac(dev);
	rtl8139_reg_irq(dev);
	rtl8139_set_int(dev);
	kprintf("rtl8139: startup complete\n");
}

// Load data from the PCI
void rtl8139_init_pci(rtl8139_dev_t* dev) {
	pci_device_t* pci_dev = dev->pci_dev;
	uint8_t header_type = pci_dev->header_type;

	if ( header_type != 0 ) {
		kprintf("rtl8139: header_type of pci must be 0, is %d\n", header_type);
		return;
	}

	pci_device_normal_t* header = (pci_device_normal_t*) pci_dev->extra;
	dev->irq = header->interrupt_line;

	#ifdef BRACKOS_CONF_ACPI
	dev->irq = acpi_irq_remap(dev->irq);
	#endif

	kprintf("rtl8139: irq %d\n", dev->irq);

	for ( int i = 0; i < 6; i++ ) {
		uint32_t base = pci_device_base_addr(dev->pci_dev, i);

		if  ( base == 0 ) {
			continue;
		}

		if ( (base & 1) == 1 ) {
			dev->io_addr = base & 0xFFFC;
			kprintf("rtl8139: io addr %p\n", dev->io_addr);
		}
		else {
			dev->mem_addr = base & 0xFFFFFFF0;
			kprintf("rtl8139: mem addr %p\n", dev->mem_addr);
		}
	}

	// Enable bus mastering
	pci_cmd_reg_t cmd_reg = pci_device_read_cmd(pci_dev);
	kprintf("cmd_reg: %x\n", cmd_reg.data);
	cmd_reg.bus_master = 1;
	pci_device_write_cmd(pci_dev, cmd_reg);
	kprintf("rtl8139: attempted to enable bus mastering\n");

	// Check it was actually enabled.
	cmd_reg = pci_device_read_cmd(pci_dev);

	if ( cmd_reg.bus_master != 1 ) {
		kprintf("rtl8139: failed to enable bus mastering\n");
		dev->io_addr = 0;
		dev->mem_addr = 0;
	}

	kprintf("rtl8139: bus mastering enabled.\n");
}

// Check if the PCI info was loaded correctly.
int rtl8139_pci_ready(rtl8139_dev_t* dev) {
	return dev->io_addr != 0 &&
		   dev->mem_addr != 0;
}

void rtl8139_init() {
	// Get the device from the PCI table
	pci_device_t* pci_dev = pci_get(RTL8139_PCI_VENDOR, RTL8139_PCI_DEVICE);

	if ( pci_dev == NULL ) {
		kprintf("rtl8139: no device found\n");
		return;
	}

	// Allocate and setup the device
	rtl8139_dev_t* dev = (rtl8139_dev_t*) kmalloc(sizeof(rtl8139_dev_t));

	if ( dev == NULL ) {
		kprintf("rtl8139: failed to alloc device\n");
		return;
	}

	MEMZERO(dev);
	dev->pci_dev = pci_dev;

	// Do the PCI setup
	rtl8139_init_pci(dev);	

	if ( !rtl8139_pci_ready(dev) ) {
		kprintf("rtl8139: failed to find io/mem address\n");
		kfree(dev);

		return;
	}

	// TODO - not this
	g_dev = dev;

	rtl8139_startup(dev);

	// Send a test packet
	char test[80];
	for ( int i = 0; i < 80; i++ ) {
		test[i] = (char) i;
	}

	rtl8139_send_packet(dev, test, 80);
}