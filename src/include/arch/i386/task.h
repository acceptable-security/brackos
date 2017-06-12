#ifndef _TASK_H
#define _TASK_H

#include <stdint.h>
#include <arch/i386/irq.h>
#include <arch/i386/paging.h>

struct task_s;
typedef struct task_s task_t;

#define TASK_STATE_PREEMPT  0 // Currently waiting to be preempted
#define TASK_STATE_RUNNNING 1 // Currently running
#define TASK_STATE_SLEEP    2 // Currently waiting for something
#define TASK_STATE_ZOMBIE   3 // Currently killed but not reaped
#define TASK_STATE_STARTED  4 // Currently started but never runned

#define TASK_STACK_SIZE 4096
#define MAX_TASKS (2 << 16) - 1

typedef int32_t pid_t;

struct task_s {
    task_t* previous;
    task_t* next;

    char* name;
    pid_t pid;
    uint32_t state;

    uintptr_t esp;
    page_directory_t* cr3;

    uintptr_t stack_bottom;
    uintptr_t stack_top;
    irq_regs_t* user_regs;

    uintptr_t int_stack_bottom;
    uintptr_t int_stack_top;

};

task_t* task_create(char* name);
task_t* task_kernel_create(char* name, uintptr_t address);
void task_schedule(task_t* task);
void task_kill(pid_t pid);
void task_dealloc(task_t* task);
void task_init(uintptr_t initial_task_fn);

#endif
