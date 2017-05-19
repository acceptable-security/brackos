#include <stdint.h>
#include <stdbool.h>
#include <arch/i386/io.h>
#include <drivers/ps2.h>


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

void ps2_flush_buffer() {
    while ( (ps2_read_status() & PS2_STAT_OUTPUT) != 0 ) {
        inportb(PS2_DATA_PORT);
    }
}

// Write command to a PS/2 device
uint8_t ps2_write_device_command(uint8_t port, uint8_t cmd) {
    if ( port == 2 ) {
        // Make sure to write to the data port and make sure the res is flushed.
        ps2_write_command(PS2_CMD_WRITE_IN_PORT_2, 0, false);
    }

    ps2_write_data(cmd);

    return ps2_read_data();
}

// Write a command to the PS/2 configuration port, and return data if necessary.
uint8_t ps2_write_command(uint8_t command, uint8_t data, bool response) {
    outportb(PS2_STAT_PORT, command);

    if ( data ) {
        ps2_write_data(data);
    }

    return response ? ps2_read_data() : 0;
}
