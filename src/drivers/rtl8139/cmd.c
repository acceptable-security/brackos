#include <arch/i386/irq.h>
#include <arch/i386/io.h>
#include <drivers/rtl8139.h>
#include <kprint.h>


void rtl8139_poweron(rtl8139_dev_t* dev) {
	outportb(dev->io_addr + RTL8139_REG_CONF1, 0x00);	
	io_wait();
	kprintf("rtl8139: device power on\n");
}

void rtl8139_softreset(rtl8139_dev_t* dev) {
	outportb(dev->io_addr + RTL8139_REG_CMD, RTL8139_CMD_RESET);
	io_wait();
	kprintf("rtl8139: sent software restart\n");

	rtl8139_cmd_t reg;

	do {
		io_wait();
		reg = (rtl8139_cmd_t) inportb(dev->io_addr + RTL8139_REG_CMD); 
	} while(reg.rst != 0);

	kprintf("rtl8139: reset complete\n");	
}

void rtl8139_load_buffer(rtl8139_dev_t* dev) {
	outportl(dev->io_addr + RTL8139_REG_RBSTART, (uintptr_t) dev->recv_buff_phys);
	io_wait();
	kprintf("rtl8139: receive buffer set\n");

	for ( int i = 0; i < 4; i++ ) {
		outportl(dev->io_addr + RTL8139_REG_TSAD + (i * 4), (uintptr_t) dev->tx_buff_phys[i]);		
		io_wait();
	}

	kprintf("rtl8139: tx buffer set\n");
}

void rtl8139_set_int(rtl8139_dev_t* dev) {
	rtl8139_imr_t reg = { .tok = 1, .rok = 1 };
	outportw(dev->io_addr + RTL8139_REG_IMR, reg.data);
	io_wait();
	kprintf("rtl8139: enabled tok and rok in imr\n");
}

void rtl8139_read_mac(rtl8139_dev_t* dev) {
	for ( int i = 0; i < 6; i++ ) {
		dev->mac[i] = inportb(dev->io_addr + RTL8139_REG_MAC + i);
	}

	kprintf("rtl8139: mac %x:%x:%x:%x:%x:%x\n",
			dev->mac[0],
			dev->mac[1],
			dev->mac[2],
			dev->mac[3],
			dev->mac[4],
			dev->mac[5]);
}

void rtl8139_set_rcr(rtl8139_dev_t* dev) {
	rtl8139_rcr_t rcr = {
		.aap = 1,
		.apm = 1,
		.am = 1,
		.ab = 1,
		.ar = 1,
		.aer = 1,
		.wrap = 1,
	};

	outportl(dev->io_addr + RTL8139_REG_RCR, rcr.data);
	io_wait();
	kprintf("rtl8139: recv all packets / wrap disabled\n");
}

void rtl8139_set_tcr(rtl8139_dev_t* dev) {
	rtl8139_tcr_t tcr = {
		.mxdma2 = 3,
		.loopback = 0b11,
		.ifg = 3
	};

	outportb(dev->io_addr + RTL8139_REG_TCR, tcr.data);
	io_wait();
	kprintf("rtl8139: tx reg set\n");
}

void rtl8139_enable_rxtx(rtl8139_dev_t* dev) {
	rtl8139_cmd_t cmd = { .re = 1, .te = 1 };
	kprintf("cmd: %x\n", cmd.data);
	outportb(dev->io_addr + RTL8139_REG_CMD, cmd.data);
	io_wait();
	kprintf("rtl8139: enabling rx & tx\n");
}

void rtl8139_reg_irq(rtl8139_dev_t* dev) {
	irq_register(dev->irq, rtl8139_interrupt);
	kprintf("rtl8139: registered irq\n");
}

rtl8139_tsd_t rtl8139_get_tsd(rtl8139_dev_t* dev, int desc) {
	uint32_t tsd_port = dev->io_addr + RTL8139_REG_TSD + (desc * 4);
	return (rtl8139_tsd_t) inportl(tsd_port);
}

rtl8139_isr_t rtl8139_get_isr(rtl8139_dev_t* dev, int ack) {
	rtl8139_isr_t isr = (rtl8139_isr_t) inportw(dev->io_addr + RTL8139_REG_ISR);

	if ( ack == 1 ) {
		outportw(dev->io_addr + RTL8139_REG_ISR, isr.data);		
	}

	return isr;
}