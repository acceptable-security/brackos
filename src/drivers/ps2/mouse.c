#include <stdint.h>
#include <arch/i386/irq.h>
#include <drivers/ps2.h>
#include <kprint.h>

uint8_t mouse_state[3];  // Mouse bytes state
uint8_t mouse_index = 0; // Index in the mouse byte state

// Read a byte out of the buffer and if possible parse the data.
void ps2_mouse_read() {
    mouse_state[mouse_index++] = ps2_read_data();

    if ( mouse_index == 3 ) {
        mouse_index = 0;

        bool left, center, right;

        uint8_t state = mouse_state[0];
        uint8_t xm = mouse_state[1];
        uint8_t ym = mouse_state[2];

        left = state & PS2_MOUSE_LEFT;
        center = state & PS2_MOUSE_CENTER;
        right = state & PS2_MOUSE_RIGHT;

        int32_t delta_x = (state & PS2_MOUSE_SIGN_X ? 0xFFFFFF00 : 0) | xm;
        int32_t delta_y = (state & PS2_MOUSE_SIGN_Y ? 0xFFFFFF00 : 0) | ym;

        if ( left )   kprintf("left ");
        if ( center ) kprintf("center ");
        if ( right )  kprintf("right ");
        kprintf("%d %d\n", delta_x, delta_y);
    }
}

// Called whenever PS/2 mouse throw an interrupt
void ps2_mouse_interrupt(irq_regs_t* frame) {
    while ( (ps2_read_status() & PS2_STAT_OUTPUT) != 0 ) {
        ps2_mouse_read();
    }
}


// After PS/2 controller setup, call this to setup PS/2 mice
bool ps2_mouse_setup(uint8_t port) {
    kprintf("ps/2 mouse up on port %d\n", port);
    if ( ps2_write_device_command(port, PS2_DEV_DEFAULTS) != PS2_RES_OK ) {
        return false;
    }

    if ( ps2_write_device_command(port, PS2_DEV_ENABLE_SCAN) != PS2_RES_OK ) {
        return false;
    }

    irq_register(12, ps2_mouse_interrupt);

    return true;
}
