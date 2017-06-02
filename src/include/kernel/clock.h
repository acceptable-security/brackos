#ifndef _CLOCK_H
#define _CLOCK_H

#include <stdbool.h>
#include <stdint.h>

struct clock_event_s;
typedef struct clock_event_s clock_event_t;

typedef void (clock_callback_t)();

typedef enum {
    CLOCK_COUNTDOWN,
    CLOCK_ONCE_OFF
} clock_type_t;

// Block in the chain to describe clock events.
struct clock_event_s {
    clock_event_t* prev;
    clock_event_t* next;

    // Callback to call
    clock_callback_t* callback;

    // Current counter
    int32_t counter;

    // Initial counter
    uint32_t initial;

    // Type of counter
    clock_type_t type;
};

void clock_init();
void clock_advance(uint32_t amount);
bool clock_add_countdown(uint32_t time, clock_callback_t* callback);
bool clock_add_once_off(uint32_t time, clock_callback_t* callback);

#endif
