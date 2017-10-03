#include <kernel/clock.h>
#include <kprint.h>

#include <stdbool.h>
#include <stdlib.h>

clock_event_t* root;

void clock_advance(uint32_t amount) {
    for ( clock_event_t* current = root; current; current = current->next ) {
        current->counter -= amount;

        if ( current->counter <= 0 ) {
            clock_callback_t* callback = current->callback;

            switch ( current->type ) {
                // Reset the counter
                case CLOCK_COUNTDOWN:
                    current->counter = current->initial;
                    break;

                // Delete the object
                case CLOCK_ONCE_OFF:
                    if ( current->prev ) {
                        current->prev->next = current->next;
                    }

                    if ( current->next ) {
                        current->next->prev = current->prev;
                    }

                    if ( current == root ) {
                        root = current->next;
                    }

                    kfree(current);

                    break;
            }

            callback();
        }
    }
}

clock_event_t* clock_add(uint32_t time, clock_callback_t* callback) {
    clock_event_t* event = (clock_event_t*) kmalloc(sizeof(clock_event_t));

    if ( event == NULL ) {
        return NULL;
    }

    // Insert into the linked list
    if ( root ) {
        event->prev = root->prev;
        event->prev->next = event;
        root->prev = event;
        event->next = root;
    }
    else {
        event->next = NULL;
        event->prev = NULL;
    }

    root = event;

    root->initial = root->counter = time;
    root->callback = callback;

    return event;
}

bool clock_add_countdown(uint32_t time, clock_callback_t* callback) {
    clock_event_t* event = clock_add(time, callback);

    if ( event == NULL ) {
        return false;
    }

    event->type = CLOCK_COUNTDOWN;

    return true;
}

bool clock_add_once_off(uint32_t time, clock_callback_t* callback) {
    clock_event_t* event = clock_add(time, callback);

    if ( event == NULL ) {
        return false;
    }

    event->type = CLOCK_ONCE_OFF;

    return true;
}


void clock_init() {
    root = NULL;
    kprintf("clock setup\n");
}
