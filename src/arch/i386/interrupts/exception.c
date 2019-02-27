#include <arch/i386/idt.h>
#include <arch/i386/apic.h>
#include <arch/i386/irq.h>
#include <kernel/scheduler.h>
#include <kprint.h>
#include <stdint.h>
#include <stdlib.h>

extern void exception_stub0();
extern void exception_stub1();
extern void exception_stub2();
extern void exception_stub3();
extern void exception_stub4();
extern void exception_stub5();
extern void exception_stub6();
extern void exception_stub7();
extern void exception_stub8();
extern void exception_stub9();
extern void exception_stub10();
extern void exception_stub11();
extern void exception_stub12();
extern void exception_stub13();
extern void exception_stub14();
extern void exception_stub15();
extern void exception_stub16();
extern void exception_stub17();
extern void exception_stub18();
extern void exception_stub19();


const char* exception_strs[] = {
    "dividing by zero",
    "debug",
    "non maskable interrupt",
    "breakpoint",
    "into detected overflow",
    "out of bounds",
    "invalid opcode",
    "no coprocessor",
    "double fault",
    "coprocessor segment overrun",
    "bad tss",
    "segment not present",
    "stack fault",
    "general protection fault",
    "page fault",
    "unknown interrupt",
    "coprocessor fault",
    "alignment check",
    "machine check"
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

void exception_handle(exc_regs_t* frame) {
    uint32_t error = frame->error_code;
    char* task_name = current_task == NULL ? "unknown task" : current_task->name;

    if ( error > 19 ) {
        kprintf("exceptions: %s threw invalid (%p) @ %p\n", task_name, error, frame);
    }
    else {
        kprintf("exceptions: %s threw %s (%p) @ %p\n", task_name, exception_strs[error], error, frame);
    }

    kprintf("cpu %d\n", lapic_get_id());

    if ( frame != NULL ) {
        kprintf("eax %p ebx %p ecx %p edx %p\n"
                "esi %p edi %p\n"
                "ebp %p esp %p\n"
                "gs %p fs %p es %p ds %p\n",
                frame->eax, frame->ebx, frame->ecx, frame->edx,
                frame->esi, frame->edi,
                frame->ebp, frame->esp,
                frame->gs, frame->fs, frame->es, frame->ds);

        exception_trace(frame->ebp, 10);
    }

    for ( ;; ) __asm__ ("hlt");
}

void exception_init() {
    idt_set_gate(0, (uintptr_t) exception_stub0, 0x08, 0x8E); 
    idt_set_gate(1, (uintptr_t) exception_stub1, 0x08, 0x8E); 
    idt_set_gate(2, (uintptr_t) exception_stub2, 0x08, 0x8E); 
    idt_set_gate(3, (uintptr_t) exception_stub3, 0x08, 0x8E); 
    idt_set_gate(4, (uintptr_t) exception_stub4, 0x08, 0x8E); 
    idt_set_gate(5, (uintptr_t) exception_stub5, 0x08, 0x8E); 
    idt_set_gate(6, (uintptr_t) exception_stub6, 0x08, 0x8E); 
    idt_set_gate(7, (uintptr_t) exception_stub7, 0x08, 0x8E); 
    idt_set_gate(8, (uintptr_t) exception_stub8, 0x08, 0x8E); 
    idt_set_gate(9, (uintptr_t) exception_stub9, 0x08, 0x8E); 
    idt_set_gate(10, (uintptr_t) exception_stub10, 0x08, 0x8E); 
    idt_set_gate(11, (uintptr_t) exception_stub11, 0x08, 0x8E); 
    idt_set_gate(12, (uintptr_t) exception_stub12, 0x08, 0x8E); 
    idt_set_gate(13, (uintptr_t) exception_stub13, 0x08, 0x8E); 
    idt_set_gate(14, (uintptr_t) exception_stub14, 0x08, 0x8E); 
    idt_set_gate(15, (uintptr_t) exception_stub15, 0x08, 0x8E); 
    idt_set_gate(16, (uintptr_t) exception_stub16, 0x08, 0x8E); 
    idt_set_gate(17, (uintptr_t) exception_stub17, 0x08, 0x8E); 
    idt_set_gate(18, (uintptr_t) exception_stub18, 0x08, 0x8E); 
    idt_set_gate(19, (uintptr_t) exception_stub19, 0x08, 0x8E); 

    kprintf("exceptions: setup complete\n");
}
