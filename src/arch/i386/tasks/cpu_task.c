#include <arch/i386/cpu_task.h>
#include <arch/i386/tss.h>
#include <mem/mmap.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>

extern void irq_save_regs(cpu_task_t* task);


#define TASK_STACK_SIZE 4096

void cpu_task_alloc(cpu_task_t* cpu_task) {
    cpu_task->int_stack_bottom = (uintptr_t) memmap(NULL, TASK_STACK_SIZE, MMAP_RW | MMAP_URGENT);

    if ( cpu_task->int_stack_bottom == 0 ) {
        kprintf("tasks: failed to create task interrupt stack\n");
        kfree(cpu_task);
        return;
    }

    cpu_task->int_stack_top = cpu_task->int_stack_bottom + TASK_STACK_SIZE;
    cpu_task->cr3 = page_directory_copy();

    memset((void*) cpu_task->int_stack_bottom, 0, TASK_STACK_SIZE);
}

void cpu_task_kernel_init(cpu_task_t* cpu_task, uintptr_t address) {
	    // Allocate a stack
    cpu_task->stack_bottom = (uintptr_t) memmap(NULL, TASK_STACK_SIZE, MMAP_RW | MMAP_URGENT);

    if ( cpu_task->stack_bottom == 0 ) {
        kprintf("tasks: failed to create stack\n");
        cpu_task_dealloc(cpu_task);
        return;
    }

    cpu_task->stack_top = cpu_task->stack_bottom + TASK_STACK_SIZE;

    memset((void*) cpu_task->stack_bottom, 0, TASK_STACK_SIZE);

    // Leave room for the IRQ registers
    cpu_task->stack_top -= sizeof(irq_regs_t);
    cpu_task->user_regs = (irq_regs_t*) cpu_task->stack_top;

    // Initialize the registers.
    cpu_task->user_regs->eflags = 0x00000202;
    cpu_task->user_regs->cs = 0x8;
    cpu_task->user_regs->eip = address;
    cpu_task->user_regs->ebp = cpu_task->stack_top;
    cpu_task->user_regs->esp = cpu_task->stack_top;
    cpu_task->user_regs->ds = 0x10;
    cpu_task->user_regs->fs = 0x10;
    cpu_task->user_regs->es = 0x10;
    cpu_task->user_regs->fs = 0x10;
}

// Schedule a task during an interrupt.
void cpu_task_schedule(cpu_task_t* cpu_task) {
    // Update tss
    tss_update(cpu_task->int_stack_top);

    paging_load_directory(cpu_task->cr3);

    irq_send_eoi(0); // TODO: load IRQ from somewhere

    // Here it goes...
    __asm__ volatile("mov %0, %%esp;"   // Load the task stack
                     "jmp irq_exit;"    // Load the new registers
    			     : :"r"(cpu_task->user_regs));
}

// Deschedule a task during an interrupt
void cpu_task_deschedule(cpu_task_t* cpu_task) {
	irq_save_regs(cpu_task);
}

void cpu_task_dealloc(cpu_task_t* cpu_task) {
    if ( cpu_task->stack_bottom ) {
        memunmap((void*) cpu_task->stack_bottom, TASK_STACK_SIZE);
    }

    if ( cpu_task->int_stack_bottom ) {
        memunmap((void*) cpu_task->int_stack_bottom, TASK_STACK_SIZE);
    }
}