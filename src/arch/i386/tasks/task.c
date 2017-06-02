#include <arch/i386/task.h>
#include <mem/mmap.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>

task_t* task_pid_table[MAX_TASKS];

// Attempts to allocate a PID. On failure, returns -1.
pid_t pid_find_empty() {
    for ( int i = 0; i < MAX_TASKS; i++ ) {
        if ( task_pid_table[i] == NULL ) {
            return -1;
        }
    }

    kprintf("tasks: pid table full\n");
    return MAX_TASKS + 1; // Impossible value
}

// A general function for creating tasks. Used by other functions.
task_t* task_create(char* name) {
    // Allocate space for the task
    task_t* task = (task_t*) kmalloc(sizeof(task_t));

    if ( task == NULL ) {
        return NULL;
    }

    memset(task, 0, sizeof(task_t));

    // Initialize the parameters
    task->name = name;
    task->pid = pid_find_empty();

    // Failed to allocate a PID
    if ( task->pid < 0 ) {
        kfree(task);
        return NULL;
    }

    // Create a stack for handling interrupts
    task->int_stack_bottom = (uintptr_t) kmalloc(TASK_STACK_SIZE);
    task->int_stack_top = task->int_stack_bottom + TASK_STACK_SIZE;

    memset((void*) task->int_stack_bottom, 0, TASK_STACK_SIZE);

    return task;
}

// Create a task for the kernel
task_t* task_kernel_create(char* name, uintptr_t address) {
    task_t* task = task_create(name);

    if ( task == NULL ) {
        return NULL;
    }

    // Allocate a stack
    task->stack_bottom = (uintptr_t) kmalloc(TASK_STACK_SIZE);
    task->stack_top = task->stack_bottom + TASK_STACK_SIZE;

    // Leave room for the IRQ registers
    task->stack_top -= sizeof(irq_regs_t);
    task->user_regs = (irq_regs_t*) task->stack_top;

    // Initialize the registers.
    task->user_regs->eflags = 0x00000202;
    task->user_regs->cs = 0x8;
    task->user_regs->eip = address;
    task->user_regs->ebp = task->stack_top;
    task->user_regs->ds = task->user_regs->fs = task->user_regs->es = task->user_regs->fs = 0x10;

    return task;
}

// Schedule a task during an interrupt.
void task_schedule(task_t* task) {
    // TODO - save old task
    // TODO - update tss
    // TODO - load new task registers
}

// Attempts to set a task into the killed state.
void task_kill(pid_t pid) {
    // Sanity checking
    if ( pid < 0 || pid > MAX_TASKS ) {
        kprintf("task: pid %d is out of range\n", pid);
        return;
    }

    task_t* task = task_pid_table[pid];

    if ( task == NULL ) {
        kprintf("task: pid %d doesn't exist.\n", pid);
        return;
    }

    if ( task->state == TASK_STATE_ZOMBIE ) {
        kprintf("task: pid %d is already dead!\n", pid);
        return;
    }

    task->state = TASK_STATE_ZOMBIE;
}

// Deallocates a task
void task_dealloc(task_t* task) {
    if ( task == NULL ) {
        return;
    }

    // Remove the task from PID table
    if ( task->pid != -1 ) {
        task_pid_table[task->pid] = NULL;
    }

    // Remove task from double linked list
    if ( task->previous ) {
        task->previous->next = task->next;
    }

    if ( task->next ) {
        task->next->previous = task->previous;
    }

    // Deallocate task resources
    if ( task->stack_bottom ) {
        kfree((void*) task->stack_bottom);
    }

    if ( task->int_stack_bottom ) {
        kfree((void*) task->int_stack_bottom);
    }

    kfree(task);
}
