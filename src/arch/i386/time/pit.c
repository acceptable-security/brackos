#include <arch/i386/io.h>
#include <arch/i386/irq.h>
#include <arch/i386/pit.h>
#include <kernel/clock.h>
#include <kprint.h>
#include <stdint.h>
uint32_t sleep_counter; // TODO - no concurrency support!! Replace soon...?
                        // Since the PIT is only active during the PIC's usage
                        // which is only used during uniprocessor modes, is there
                        // ever a scenerio where that could be in conflict?
uint32_t ms_per_tick;

// Called every IRQ 0
void pit_handle_interrupt(irq_regs_t* frame) {
    clock_advance(ms_per_tick);

    if ( sleep_counter > 0 ) {
        sleep_counter--;
    }
    // TODO - callbacks? (e.g. every X ms call Y)
}

// Send command to the PIT
void pit_cmd_send(pit_cmd_t data) {
    // Cast the packed struct to an integer and send to PIT_CMD_CHAN
    outportb(PIT_CMD_CHAN, *(uint8_t*)(&data));
    io_wait();
}

// Read data from a channel
uint8_t pit_data_read(uint8_t channel) {
    switch ( channel ) {
        case 0:
            return inportb(PIT_DATA_CHAN_0);
        case 1:
            return inportb(PIT_DATA_CHAN_1);
        case 2:
            return inportb(PIT_DATA_CHAN_2);
        default:
            return 0;
    }
}

// Send data to a channel
void pit_data_send(uint8_t channel, uint8_t data) {
    switch ( channel ) {
        case 0:
            outportb(PIT_DATA_CHAN_0, data);
            break;

        case 1:
            outportb(PIT_DATA_CHAN_1, data);
            break;

        case 2:
            outportb(PIT_DATA_CHAN_2, data);
            break;

    }

    io_wait();
}

void pit_sleep(uint32_t ms) {
    if ( sleep_counter != 0 ) {
        kprintf("Uh oh... PIT received a request for sleep while it wasn't done.");
        return;
    }

    sleep_counter = ms / ms_per_tick;

    kprintf("Waiting for %d ticks...\n", sleep_counter);

    while ( sleep_counter > 0 ) {
        kprintf("");
    }
}

// Start a basic square wave interrupt.
void pit_start(uint16_t frequency) {
    if ( frequency == 0 ) {
        return;
    }

    // Calculate the frequency divider.
    uint16_t divisor = PIT_BASE_FREQ / frequency;

    // Send the initial configuration command.
    pit_cmd_send((pit_cmd_t) {
        .bin_mode       = PIT_MODE_BINARY,
        .operating_mode = PIT_OP_MODE_SQUARE_GEN,
        .access_mode    = PIT_ACC_MODE_LOHIBYTE,
        .select_channel = PIT_SEL_CHAN_0
    });

    // Send the low then high byte of the frequency divider.
    pit_data_send(0, divisor & 0xFF);
    pit_data_send(0, (divisor >> 8) & 0xFF);

    ms_per_tick = 1000 / frequency;
}

void pit_init() {
    irq_register(0, pit_handle_interrupt);
    pit_start(500); // 2ms per tick

    sleep_counter = 0;

    kprintf("pit setup\n");
}
