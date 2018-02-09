#include <arch/i386/io.h>
#include <drivers/ps2.h>

#include <kprint.h>

#include <stdint.h>
#include <stdbool.h>

bool ps2_first_device = true;   // Boolean for whether or not the first PS/2 port has a device present
bool ps2_second_device = false; // Boolean for whether or not the second PS/2 port has a device present

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
    ps2_flush_buffer();
    
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
    ps2_flush_buffer();

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
    ps2_flush_buffer();

    uint8_t response, selftest;
    bool success = true;

    if ( ps2_first_device ) {
        // Write reset command and test for command success
        response = ps2_write_device_command(1, PS2_DEV_RESET_PORT);
        selftest = ps2_read_data();

        success = (response == PS2_RES_OK && selftest == PS2_RES_TESTOK);
    }

    if ( ps2_second_device ) {
        response = ps2_write_device_command(2, PS2_DEV_RESET_PORT);
        selftest = ps2_read_data();

        success = success || (response == PS2_RES_OK && selftest == PS2_RES_TESTOK);
    }

    return success;
}

// Return the device type byte(s)
uint16_t ps2_get_device_type(uint8_t port) {
    ps2_flush_buffer();

    if ( ps2_write_device_command(port, PS2_DEV_DISABLE_SCAN) != PS2_RES_OK ) {
        kprintf("ps/2 failed to disable scanning\n");
        return 0xFF;
    }

    if ( ps2_write_device_command(port, PS2_DEV_IDENTIFY) != PS2_RES_OK ) {
        kprintf("ps/2 failed to identify\n");
        return 0xFF;
    }

    uint16_t res = (uint16_t) ps2_read_data();

    if ( res == 0xAB ) {
        res = (res << 8) | (uint16_t)(ps2_read_data());
    }

    kprintf("ps2 port %d: %x\n", port, res);

    return res;
}

// Setup the PS/2 Controller
bool ps2_setup() {
    // Disable the devices
    ps2_toggle_port(1, false);
    ps2_toggle_port(2, false);

    // Flush the device buffer
    ps2_flush_buffer();

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
    if ( ps2_first_device )  ps2_toggle_port(1, true);
    if ( ps2_second_device ) ps2_toggle_port(2, true);

    // Enable the interrupts
    conf = ps2_read_configuration();
    conf |= (ps2_first_device  ? PS2_CONF_INT1 : 0);
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

// Run the individual device setups
bool ps2_setup_devices() {
    bool success = true;

    if ( ps2_first_device ) {
        switch ( ps2_get_device_type(1) ) {
            case PS2_TYPE_MF2_KB_TRANS1:
            case PS2_TYPE_MF2_KB_TRANS2:
            case PS2_TYPE_MF2_KB:
                success = success && ps2_kb_setup(1);
                break;

            case PS2_TYPE_MOUSE:
            case PS2_TYPE_MOUSE_SCROLL:
            case PS2_TYPE_MOUSE_5BTN:
                success = success && ps2_mouse_setup(1);
                break;
        }
    }

    if ( ps2_second_device ) {
        switch ( ps2_get_device_type(2) ) {
            case PS2_TYPE_MF2_KB_TRANS1:
            case PS2_TYPE_MF2_KB_TRANS2:
            case PS2_TYPE_MF2_KB:
                success = success && ps2_kb_setup(2);
                break;

            case PS2_TYPE_MOUSE:
            case PS2_TYPE_MOUSE_SCROLL:
            case PS2_TYPE_MOUSE_5BTN:
                success = success && ps2_mouse_setup(2);
                break;
        }
    }

    return success;
}
