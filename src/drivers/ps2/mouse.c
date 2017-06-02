#include <stdint.h>
#include <arch/i386/irq.h>
#include <drivers/ps2.h>
#include <kprint.h>

// Called whenever PS/2 mouse throw an interrupt
void ps2_mouse_interrupt(irq_regs_t* frame) {
    bool left, center, right;

    uint8_t state = ps2_read_data();
    uint8_t xm = ps2_read_data();
    uint8_t ym = ps2_read_data();

    left = state & PS2_MOUSE_LEFT;
    center = state & PS2_MOUSE_CENTER;
    right = state & PS2_MOUSE_RIGHT;

    // Im 99% sure this is completely incorrect maths.
    int16_t delta_x = xm + ((state >> 6) & 1);
    delta_x = (state & PS2_MOUSE_SIGN_X) ? -delta_x : delta_x;

    int16_t delta_y = ym + ((state >> 7) & 1);
    delta_y = (state & PS2_MOUSE_SIGN_Y) ? -delta_y : delta_y;

    if ( left ) kprintf("left ");
    if ( center ) kprintf("center ");
    if ( right ) kprintf("right ");
    kprintf("%d %d\n", delta_x, delta_y);

    ps2_flush_buffer();
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
