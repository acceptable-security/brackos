#include <kernel/scheduler.h>
#include <kernel/task.h>
#include <mem/mmap.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>
#include <list.h>

task_t* task_pid_table[MAX_TASKS];

// Attempts to allocate a PID. On failure, returns -1.
pid_t pid_find_empty() {
    for ( int i = 0; i < MAX_TASKS; i++ ) {
        if ( task_pid_table[i] == NULL ) {
            return i;
        }
    }

    kprintf("tasks: pid table full\n");
    return -1;
}

// A general function for creating tasks. Used by other functions.
task_t* task_create(char* name) {
    // Allocate space for the task
    task_t* task = (task_t*) kmalloc(sizeof(task_t));

    if ( task == NULL ) {
        kprintf("tasks: failed to allocate task\n");
        return NULL;
    }

    memset(task, 0, sizeof(task_t));

    // Initialize the parameters
    task->name = name;
    task->pid = pid_find_empty();
    task->state = TASK_STATE_STARTED;

    // Failed to allocate a PID
    if ( task->pid < 0 ) {
        kfree(task);
        return NULL;
    }

    cpu_task_alloc(&task->cpu);

    task->files.max_fid = 0;
    list_init(&task->files.pairs);

    return task;
}

// Create a task for the kernel
task_t* task_kernel_create(char* name, uintptr_t address) {
    kprintf("tasks: creating %s @ %p\n", name, address);
    task_t* task = task_create(name);

    if ( task == NULL ) {
        kprintf("tasks: failed to make kernel task\n");
        return NULL;
    }

    cpu_task_kernel_init(&task->cpu, address);

    return task;
}

// Attempts to set a task into the killed state.
void task_kill(pid_t pid) {
    // Sanity checking
    if ( pid < 0 || pid > MAX_TASKS ) {
        kprintf("tasks: pid %d is out of range\n", pid);
        return;
    }

    task_t* task = task_pid_table[pid];

    if ( task == NULL ) {
        kprintf("tasks: pid %d doesn't exist.\n", pid);
        return;
    }

    if ( task->state == TASK_STATE_ZOMBIE ) {
        kprintf("tasks: pid %d is already dead!\n", pid);
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
    cpu_task_dealloc(&task->cpu);

    kfree(task);
}

void task_reaper() {
    int i = 0;

    while ( 1 ) {
        i++;
    }  
}

// Initialize the tasking system.
void task_init(uintptr_t initial_task_fn) {
    for ( int i = 0; i < MAX_TASKS; i++ ) {
        task_pid_table[i] = NULL;
    }

    task_t* initial_task = task_kernel_create("init", initial_task_fn);

    if ( initial_task == NULL ) {
        kprintf("tasks: failed to make initial task\n");
        return;
    }


    kprintf("tasks: priming the scheduler...\n");
    scheduler_init(initial_task);

    kprintf("tasks: intialized\n");
}
