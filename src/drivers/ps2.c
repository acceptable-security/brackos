// TODO - device independent I/O?
// Device controller support for the 8042 PS/2 controller.
#include <arch/i386/irq.h>
#include <arch/i386/pic.h>
#include <arch/i386/apic.h>
#include <arch/i386/io.h>
#include <drivers/scancodes.h>
#include <drivers/ps2.h>
#include <kprint.h>
#include <stdbool.h>
#include <stdint.h>


bool ps2_first_device = true;   // Boolean for whether or not the first PS/2 port has a device present
bool ps2_second_device = false; // Boolean for whether or not the second PS/2 port has a device present

// Read the PS/2 controller status byte
uint8_t ps2_read_status() {
    return inportb(PS2_STAT_PORT);
}

// Wait for the PS/2 controller to be ready for writing
void ps2_wait_output() {
    while ( (ps2_read_status() & PS2_STAT_OUTPUT) == 0 );
}

// Wait for the PS/2 controller to be ready for reading
void ps2_wait_input() {
    while ( (ps2_read_status() & PS2_STAT_INPUT) != 0 );
}

// Read the PS/2 controller data byte
uint8_t ps2_read_data() {
    ps2_wait_output();
    return inportb(PS2_DATA_PORT);
}

// Write data to the PS/2 data port.
void ps2_write_data(uint8_t data) {
    ps2_wait_input();
    outportb(PS2_DATA_PORT, data);
}

// Write command to a PS/2 device
uint8_t ps2_write_device_command(uint8_t port, uint8_t cmd) {
    if ( port == 2 ) {
        // Make sure to write to the data port and make sure the res is flushed.
        ps2_write_data(PS2_CMD_WRITE_IN_PORT_2);
        ps2_read_data();
    }

    ps2_write_data(cmd);

    return ps2_read_data();
}

// Write a command to the PS/2 configuration port, and return data if necessary.
uint8_t ps2_write_command(uint8_t command, uint8_t data, bool response) {
    outportb(PS2_STAT_PORT, command);
    io_wait();

    if ( data ) {
        ps2_write_data(data);
    }

    return response ? ps2_read_data() : 0;
}

// Read the controller configuration byte
uint8_t ps2_read_configuration() {
    return ps2_write_command(PS2_CMD_READ_CONF, 0, true);
}

// Write the controller configuration byte
void ps2_write_configuration(uint8_t byte) {
    ps2_write_command(PS2_CMD_WRITE_CONF, byte, false);
}

// Enable/Disable the first or second device.
void ps2_toggle_port(uint8_t port, bool toggle) {
    if ( port == 1 ) {
        ps2_write_command(toggle ? PS2_CMD_ENABLE_PORT1 : PS2_CMD_DISABLE_PORT1, 0, false);
    }
    else if ( port == 2 ) {
        ps2_write_command(toggle ? PS2_CMD_ENABLE_PORT2 : PS2_CMD_DISABLE_PORT2, 0, false);
    }
}

// Request that the PS/2 controller do a self test.
bool ps2_self_test() {
    return ps2_write_command(PS2_CMD_SELF_TEST, 0, true) == 0x55;
}

// Request the PS/2 controller to test the attached devices
bool ps2_test_devices() {
    bool success = (ps2_write_command(PS2_CMD_TEST_PORT1, 0, true) == 0x00);
    ps2_first_device = success;

    if ( ps2_second_device ) {
        // If the first device fails, the second device can still be used.
        ps2_second_device = (ps2_write_command(PS2_CMD_TEST_PORT2, 0, true) == 0x00);
        success = success || ps2_second_device;
    }

    return success;
}

// Reset all enabled devices
bool ps2_reset_devices() {
    uint8_t response, selftest;
    bool success;

    if ( ps2_first_device ) {
        // Write reset command and test for command success
        response = ps2_write_device_command(1, PS2_DEV_RESET_PORT);
        selftest = ps2_read_data();

        success = (response == 0xFA && selftest == 0xAA);
    }

    if ( ps2_second_device ) {
        response = ps2_write_device_command(2, PS2_DEV_RESET_PORT);
        selftest = ps2_read_data();

        success = success || (response == 0xFA && selftest == 0xAA);
    }

    return success;
}

// Return the device type byte(s)
uint16_t ps2_get_device_type(uint8_t port) {
    if ( ps2_write_device_command(port, PS2_DEV_DISABLE_SCAN) != 0xFA ) {
        return 0;
    }

    if ( ps2_write_device_command(port, PS2_DEV_IDENTIFY) != 0xFA ) {
        return 0;
    }

    uint16_t res = (uint16_t) ps2_read_data();

    if ( res == 0xAB ) {
        res = (res << 8) | (uint16_t)(ps2_read_data());
    }

    return res;
}

// Setup the PS/2 Controller
bool ps2_setup() {
    // Disable the devices
    ps2_toggle_port(1, false);
    ps2_toggle_port(2, false);

    // Flush the device buffer
    inportb(PS2_DATA_PORT);

    // Disable interrupts and keyboard translation.
    uint8_t conf = ps2_read_configuration();
    conf &= ~PS2_CONF_INT1;
    conf &= ~PS2_CONF_INT2;
    conf &= ~PS2_CONF_TRANSL;
    ps2_write_configuration(conf);

    // Check for bit 5 for possibility of second device
    ps2_second_device = (conf & PS2_CONF_CLOCK2) != 0;

    // Run a self test
    if ( !ps2_self_test() ) {
        kprintf("ps2 failed self test\n");
        return false;
    }

    // Check for a second device
    if ( ps2_second_device ) {
        // Try to enable the second device
        ps2_toggle_port(2, true);
        io_wait();

        // Reread the configuration byte
        conf = ps2_read_configuration();

        // If the device 2 clock is still enabled, theres no second device
        if ( conf & PS2_CONF_CLOCK2 ) {
            ps2_second_device = false;
        }
        else {
            // Disable the second device if it does exist.
            ps2_toggle_port(2, false);
        }
    }

    // Have the PS/2 controller do checks on the devices
    if ( !ps2_test_devices() ) {
        kprintf("ps2 failed device tests\n");
        return false;
    }

    // Re-enable the devices
    if ( ps2_first_device ) ps2_toggle_port(1, true);
    if ( ps2_second_device ) ps2_toggle_port(2, true);

    // Enable the interrupts
    conf = ps2_read_configuration();
    conf |= (ps2_first_device ? PS2_CONF_INT1 : 0);
    conf |= (ps2_second_device ? PS2_CONF_INT2 : 0);
    conf |= PS2_CONF_TRANSL;
    ps2_write_configuration(conf);

    // Reset the devices
    if ( !ps2_reset_devices() ) {
        kprintf("ps2 failed reset devices\n");
        return false;
    }

    // Success!
    return true;
}

// Called whenever PS/2 keyboards throw an interrupt
void ps2_kb_interrupt(idt_reg_stack_t* frame) {
    uint8_t scancode = ps2_read_data();
    bool down = !(scancode & 0x80);
    scancode = scancode & ~0x80;

    uint8_t ascii = kb_us[scancode];

    if ( ascii > 0x7F ) {
        // TODO
    }
    else {
        // TODO
    }
}

// Called whenever PS/2 mouse throw an interrupt
void ps2_mouse_interrupt(idt_reg_stack_t* frame) {
    uint8_t byte1 = ps2_read_data();
    uint8_t byte2 = ps2_read_data();

    // TODO again
}

// Initialize the PS/2 keyboard
void ps2_init() {
    if ( !ps2_setup() ) {
        return;
    }

    if ( ps2_first_device ) {
        kprintf("device 1: %x\n", ps2_get_device_type(1));
    }

    // TODO - don't assume, why is device 2 showing up as 0xAB 0x41???
    if ( ps2_write_device_command(2, 0xF4) == 0xFA ) {
        kprintf("enabled mouse\n");
    }

    if ( ps2_second_device ) {
        kprintf("device 2: %x\n", ps2_get_device_type(2));
    }

    irq_register(1, ps2_kb_interrupt);
    irq_register(12, ps2_mouse_interrupt);

    kprintf("ps/2 controller enabled.\n");
}
