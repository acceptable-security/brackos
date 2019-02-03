#ifndef _CPU_TASK
#define _CPU_TASK

#include <stdint.h>
#include <arch/i386/irq.h>
#include <arch/i386/paging.h>

typedef struct {
    uintptr_t esp;
    page_directory_t* cr3;

    uintptr_t stack_bottom;
    uintptr_t stack_top;
    irq_regs_t* user_regs;

    uintptr_t int_stack_bottom;
    uintptr_t int_stack_top;
} cpu_task_t;

void cpu_task_alloc(cpu_task_t* cpu_task);
void cpu_task_kernel_init(cpu_task_t* cpu_task, uintptr_t address);
void cpu_task_schedule(cpu_task_t* cpu_task);
void cpu_task_deschedule(cpu_task_t* cpu_task);
void cpu_task_dealloc(cpu_task_t* cpu_task);

#endif