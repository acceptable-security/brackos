#ifndef _PS2_H
#define _PS2_H

#include <stdbool.h>
#include <stdint.h>
#include <arch/i386/idt.h>

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

// Generic PS/2 commands for devices
#define PS2_DEV_IDENTIFY      0xF2 // Identify device
#define PS2_DEV_SET_RATE      0xF3 // Set the rate of the device.
#define PS2_DEV_ENABLE_SCAN   0xF4 // Enable scanning
#define PS2_DEV_DISABLE_SCAN  0xF5 // Disable scanning
#define PS2_DEV_DEFAULTS      0xF6 // Use default settings
#define PS2_DEV_RESEND        0xFE // Resend the last packet
#define PS2_DEV_RESET_PORT    0xFF // Reset the port.

// Commands for the PS/2 keyboard
#define PS2_KEYBOARD_SET_LED 0xED // Set the LED state
#define PS2_KEYBOARD_EHCO    0xEE // Echo.

// Commands for the PS/2 mouse
#define PS2_MOUSE_SET_WRAP   0xEE // Set Wrap Mode
#define PS2_MOUSE_RESET_WRAP 0xEC // Reset Wrap Mode
#define PS2_MOUSE_READ_DATA  0xEB // Read Data
#define PS2_MOUSE_SET_STREAM 0xEA // Set Stream Mode
#define PS2_MOUSE_STATUS_REQ 0xE9 // Status Request
#define PS2_MOUSE_SET_RES    0xE8 // Set Resolution

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

// Bits in the PS/2 Mouse state byte
#define PS2_MOUSE_LEFT       (1 << 0)
#define PS2_MOUSE_CENTER     (1 << 1)
#define PS2_MOUSE_RIGHT      (1 << 2)
#define PS2_MOUSE_SIGN_X     (1 << 4)
#define PS2_MOUSE_SIGN_Y     (1 << 5)
#define PS2_MOUSE_OVERFLOW_X (1 << 6)
#define PS2_MOUSE_OVERFLOW_Y (1 << 7)

// Different responses that the PS/2 keyboard sends
#define PS2_RES_OK     0xFA // Everything is O
#define PS2_RES_RESEND 0xFE // Resend the command
#define PS2_RES_TESTOK 0xAA // Test was successful
// Everything else can be assumed to be a failure.

// Different types of devices that the PS/2 controller can handle
#define PS2_TYPE_MOUSE         0x0000 // Standard PS/2 mouse
#define PS2_TYPE_MOUSE_SCROLL  0x0003 // PS/2 mouse with scroll
#define PS2_TYPE_MOUSE_5BTN    0x0004 // 5 button mouse
#define PS2_TYPE_MF2_KB_TRANS1 0xAB41 // MF2 keyboard with translation
#define PS2_TYPE_MF2_KB_TRANS2 0xABC1 // MF2 keyboard with translation
#define PS2_TYPE_MF2_KB        0xAB83 // MF2 keyboard

uint8_t ps2_read_status();
uint8_t ps2_read_data();
void ps2_write_data(uint8_t data);
void ps2_flush_buffer();
uint8_t ps2_write_device_command(uint8_t port, uint8_t cmd);
uint8_t ps2_write_command(uint8_t command, uint8_t data, bool response);

uint8_t ps2_read_configuration();
void ps2_write_configuration(uint8_t byte);
void ps2_toggle_port(uint8_t port, bool toggle);
bool ps2_self_test();
bool ps2_test_devices();
bool ps2_reset_devices();
uint16_t ps2_get_device_type(uint8_t port);
bool ps2_setup();
bool ps2_setup_devices();

void ps2_kb_interrupt(irq_regs_t* frame);
bool ps2_kb_setup(uint8_t port);

void ps2_mouse_interrupt(irq_regs_t* frame);
bool ps2_mouse_setup(uint8_t port);

void ps2_init();

#endif
