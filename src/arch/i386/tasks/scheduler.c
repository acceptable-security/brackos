#include <arch/i386/irq.h>
#include <arch/i386/scheduler.h>
#include <arch/i386/task.h>
#include <kernel/clock.h>
#include <kprint.h>

bool first_run = true;
task_t* current_task;

// Simple round robin for now
void scheduler_advance() {
    // We always must be advancing during an IRQ
    if ( !irq_is_happening() ) {
        return;
    }

    task_t* previous_task = current_task;
    current_task = current_task->next;

    // Don't bother wasting time
    if ( current_task != previous_task || current_task->state == TASK_STATE_STARTED ) {
        // Set the task state to preempted
        current_task->state = TASK_STATE_PREEMPT;

        if ( !first_run ) {
            // Save the old stack
            previous_task->user_regs = irq_get_current_regs();
        }
        else {
            first_run = false;
        }

        // Load the new one
        task_schedule(current_task);
    }
}

// Add a task to the scheduler
void scheduler_add(task_t* task) {
    task->next = current_task->next;
    current_task->next = task;
}

// Initialize the scheduler with the first ask
void scheduler_init(task_t* first) {
    // Initialize the task into a single circular linked list
    current_task = first;
    current_task->next = current_task;

    // Each process gets 10ms. This may actually be too much.
    clock_add_countdown(10, scheduler_advance);
}
