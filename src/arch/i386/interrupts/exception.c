#include <arch/i386/idt.h>
#include <arch/i386/apic.h>
#include <arch/i386/irq.h>
#include <kernel/scheduler.h>
#include <kprint.h>
#include <stdint.h>
#include <stdlib.h>

extern void exception_common_stub();

const char* exception_strs[] = {
    "Dividing by Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Uknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check"
};

void exception_trace(uintptr_t _ebp, size_t frames) {
    uintptr_t* ebp = (uintptr_t*) _ebp;
    kprintf("stack trace (from %p):\n", _ebp);

    for ( size_t i = 0; i < frames; i++ ) {
        if ( ebp == NULL ) {
            break;
        }

        uintptr_t eip = ebp[1];
        
        if (eip == 0 ) {
            break;
        } 

        kprintf("0x%p:0x%p\n", eip, ebp);
        ebp = (uintptr_t*) ebp[0];
    }
} 

void exception_handle(exc_regs_t* frame, int error) {
    char* task_name = current_task == NULL ? "unknown task" : current_task->name;

    if ( error > 19 ) {
        kprintf("exceptions: invalid (%d) @ %p\n", error, frame);
    }
    else {
        kprintf("exceptions: %s threw %s @ %p\n", task_name, exception_strs[error], frame);
    }

    kprintf("cpu %d\n", lapic_get_id());

    if ( frame != NULL ) {
        kprintf("eax %p ebx %p ecx %p edx %p\n"
                "esi %p edi %p\n"
                "ebp %p esp %p\n",
                frame->eax, frame->ebx, frame->ecx, frame->edx,
                frame->esi, frame->edi,
                frame->ebp, frame->esp);

        exception_trace(frame->ebp, 10);
    }

    for ( ;; ) __asm__ ("hlt");
}

void exception_init() {
    for ( int i = 0; i < 19; i++ ) {
        idt_set_gate(i, (uintptr_t) exception_common_stub, 0x08, 0x8E);
    }

    kprintf("exceptions: setup complete\n");
}
