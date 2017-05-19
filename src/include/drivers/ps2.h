#include <stdbool.h>
#include <stdint.h>

// Ports used in the working of the PS/2 controller and devices.
#define PS2_DATA_PORT 0x60 // Port for PS/2 data I/O
#define PS2_STAT_PORT 0x64 // Read for status, write for command

#define PS2_CMD_READ_CTRL_PORT   0xD0 // Command to read from PS/2 Controller output buffer
#define PS2_CMD_WRITE_CTRL_PORT  0xD1 // Command to write from PS/2 Controller output buffer
#define PS2_CMD_WRITE_OUT_PORT_1 0xD2 // Command to write to port 1 output buffer
#define PS2_CMD_WRITE_OUT_PORT_2 0xD3 // Command to write to port 2 output buffer
#define PS2_CMD_WRITE_IN_PORT_2  0xD4 // Command to write to port 2 input buffer

// Commands that can be written to the PS/2 command port.
#define PS2_CMD_READ_CONF     0x20 // Read the first byte from internal RAM - Controller Configuration Byte
#define PS2_CMD_WRITE_CONF    0x60 // Write the first byte form internal RAM - Controller Configuration Byte
#define PS2_CMD_DISABLE_PORT2 0xA7 // Disable the second PS/2 port
#define PS2_CMD_ENABLE_PORT2  0xA8 // Enable the second PS/2 port
#define PS2_CMD_TEST_PORT2    0xA9 // Test the second PS/2 port. 0x00 on success.
#define PS2_CMD_SELF_TEST     0xAA // Test the PS/2 controller. 0x55 on success.
#define PS2_CMD_TEST_PORT1    0xAB // Test the first PS/2 port. 0x00 on success.
#define PS2_CMD_DISABLE_PORT1 0xAD // Disable the first PS/2 port
#define PS2_CMD_ENABLE_PORT1  0xAE // Enable the first PS/2 port
#define PS2_CMD_RESET_CPU     0xFE // Reset the CPU. For some reason...

#define PS2_DEV_IDENTIFY      0xF2 // Identify device
#define PS2_DEV_DISABLE_SCAN  0xF5 // Disable scanning
#define PS2_DEV_RESET_PORT    0xFF // Reset the port.

// Bits in the PS/2 controller configuration byte.
#define PS2_CONF_INT1   (1 << 0) // First PS/2 port interrupt. 1 if set.
#define PS2_CONF_INT2   (1 << 1) // Second PS/2 port interrupt. 2 if set.
#define PS2_CONF_SYS    (1 << 2) // System flag. Set when the computer passes the POST.
#define PS2_CONF_CLOCK1 (1 << 4) // First PS/2 port clock
#define PS2_CONF_CLOCK2 (1 << 5) // Second PS/2 port clock
#define PS2_CONF_TRANSL (1 << 6) // First PS/2 port translation

// Bits in the PS/2 status register
#define PS2_STAT_OUTPUT      (1 << 0) // Has data to be read
#define PS2_STAT_INPUT       (1 << 1) // Is ready to receive data
#define PS2_STAT_SYSTEM      (1 << 2) // Cleared if system passes POST
#define PS2_STAT_CMD_SWITCH  (1 << 3) // 0 == data, 1 == command is written to input buffer
#define PS2_STAT_TIMEOUT_ERR (1 << 6) // Timeout Error
#define PS2_STAT_PARITY_ERR  (1 << 7) // Partiy Error

// Bits in the PS/2 Output byte
#define PS2_OUT_SYSRESET (1 << 0) // System Reset. DO NOT TOUCH!
#define PS2_OUT_A20GATE  (1 << 1) // A20 Gate.
#define PS2_OUT_CLOCK2   (1 << 2) // Second PS/2 port clock
#define PS2_OUT_DATA2    (1 << 3) // Second PS/2 port data
#define PS2_OUT_BUFF1    (1 << 4) // First PS/2 device has data
#define PS2_OUT_BUFF2    (1 << 5) // Second PS/2 device has data
#define PS2_OUT_CLOCK1   (1 << 6) // First PS/2 port clock
#define PS2_OUT_DATA1    (1 << 7) // First PS/2 clock data

uint8_t ps2_read_status();
uint8_t ps2_read_data();
void ps2_write_data(uint8_t data);
uint8_t ps2_write_command(uint8_t command, uint8_t data, bool response);
uint8_t ps2_read_configuration();
void ps2_write_configuration(uint8_t byte);
void ps2_toggle_port(uint8_t port, bool toggle);
bool ps2_self_test();
bool ps2_test_devices();
bool ps2_reset_devices();
bool ps2_setup();
void ps2_init();
