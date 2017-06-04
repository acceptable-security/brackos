#include <arch/i386/io.h>
#include <drivers/rs232.h>
#include <kprint.h>
#include <stdbool.h>

void rs232_port_set_baud(uint16_t port, uint16_t baud) {
    uint16_t divisor = RS232_DEFAULT_BAUD / baud;

    // Cache LCR
    uint8_t lcr = inportb(port + RS232_IO_LCR);

    // Enable DLAB. 0x80 is a magic mix of flags which I will not explain.
    outportb(port + RS232_IO_LCR, 0x80);

    // Send divisor
    outportb(port + RS232_IO_BAUD_LO, divisor & 0xFF);
    outportb(port + RS232_IO_BAUD_HI, (divisor >> 8) & 0xFF);

    // Restore LCR
    outportb(port + RS232_IO_LCR, lcr);
}

// Returns whether or not the RS232 port has data in its input buffers
bool rs232_data_received(uint16_t port) {
    return (inportb(port + RS232_IO_LSR) & RS232_LSR_DR) == 0;
}

// Returns whether or not the RS232 port has data in its outport buffers
bool rs232_transmit_ready(uint16_t port) {
    return (inportb(port + RS232_IO_LSR) & RS232_LSR_TEMT) != 0;
}

// Write a single byte to a RS232 port
void rs232_write_single(uint16_t port, uint8_t data) {
    // Wait for the previous write to be complete
    while ( !rs232_transmit_ready(port) );

    // Write
    outportb(port, data);
}

// Read a single byte to a RS232 port
uint8_t rs232_read_single(uint16_t port) {
    // Wait for there to be data to read
    while ( !rs232_data_received(port) );

    // Read the data
    return inportb(port);
}

// Initialize an RS232 port
void rs232_port_init(uint16_t port) {
    // Disable interrupts
    outportb(port + RS232_IO_INTR, 0x00);

    // Set baud to 38400
    rs232_port_set_baud(port, 38400);

    // Enable FIFO, clear them, with 14-byte threshold
    outportb(port + RS232_IO_INT_IDENT, 0xC7);

    // IRQs enabled, RTS/DSR set
    outportb(port + RS232_IO_MCR, 0x0B);
}

// Initialize RS232 controller
void rs232_init() {
    rs232_port_init(RS232_PORT_COM1);
    kprintf("rs232 setup\n");
}
