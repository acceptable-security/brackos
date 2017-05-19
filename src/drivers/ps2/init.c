// Device controller support for the 8042 PS/2 controller.
#include <drivers/ps2.h>
#include <kprint.h>

void ps2_init() {
    if ( !ps2_setup() ) {
        return;
    }

    if ( !ps2_setup_devices() ) {
        return;
    }

    kprintf("ps/2 controller enabled.\n");
}
