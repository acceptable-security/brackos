#include <arch/i386/irq.h>
#include <arch/i386/scheduler.h>
#include <arch/i386/task.h>
#include <kernel/clock.h>
#include <kprint.h>

task_t* current_task;

// Simple round robin for now
void scheduler_advance() {
    kprintf("scheduler: advancing\n");

    // We always must be advancing during an IRQ
    if ( !irq_is_happening() ) {
        return;
    }

    task_t* previous_task = current_task;
    current_task = current_task->next;

    // Don't bother wasting time
    if ( current_task != previous_task ) {
        kprintf("scheduler: going to next task\n");
        // Save the old stack
        current_task->esp = irq_get_current_regs()->esp;
        kprintf("scheduler: saved old stack\n");

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
    current_task = first;
    current_task->next = current_task;
    clock_add_countdown(50, scheduler_advance);
}
