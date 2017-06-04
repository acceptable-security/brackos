#include <stdbool.h>
#include <stdint.h>

#define RS232_PORT_COM1 0x3F8
#define RS232_PORT_COM2 0x2F8
#define RS232_PORT_COM3 0x3E8
#define RS232_PORT_COM4 0x2E8

#define RS232_IO_DATA      0 // R/W Data register
#define RS232_IO_INTR      1 // Interrupt Enable Register
#define RS232_IO_INT_IDENT 2 // Interrupt Identification and FIFO Control Register
#define RS232_IO_LCR       3 // Line Control Register
#define RS232_IO_MCR       4 // Modem Control Register
#define RS232_IO_LSR       5 // Line Status Register
#define RS232_IO_MSR       6 // Modem Status Register
#define RS232_IO_SCRATCH   7 // Scratch Register

#define RS232_IO_BAUD_LO 0 // DLAB must be set - Set the low byte of the baud divisor
#define RS232_IO_BAUD_HI 1 // DLAB must be set - Set the high byte of the baud divisor

#define RS232_DEFAULT_BAUD 115200

#define RS232_LSR_DR   (1 << 0) // Data Ready
#define RS232_LSR_OE   (1 << 1) // Overrun error
#define RS232_LSR_PE   (1 << 2) // Parity Error
#define RS232_LSR_FE   (1 << 3) // Framing Error
#define RS232_LSR_BI   (1 << 4) // Break indicator
#define RS232_LSR_THRE (1 << 5) // Thransmitter Holding Register Empty
#define RS232_LSR_TEMT (1 << 6) // Transmitter Empty
#define RS232_LSR_EFFO (1 << 7) // Error in RX FIFO

void rs232_port_set_baud(uint16_t port, uint16_t baud);
bool rs232_data_received(uint16_t port);
bool rs232_transmit_ready(uint16_t port);
void rs232_write_single(uint16_t port, uint8_t data);
uint8_t rs232_read_single(uint16_t port);
void rs232_port_init(uint16_t port);
void rs232_init();
