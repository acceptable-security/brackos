#include <arch/i386/scheduler.h>
#include <arch/i386/task.h>

task_t* current_task;

// Simple round robin for now
void scheduler_advance() {
    task_t* previous_task = current_task;
    current_task = current_task->next;

    // Don't bother wasting time
    if ( current_task != previous_task ) {
        task_schedule(current_task);
    }
}

// Initialize the scheduler with the first ask
void scheduler_init(task_t* first) {
    current_task = first;
}

// TODO - clocks
// TODO - register clock quanta
// TODO - call scheduler_advance
