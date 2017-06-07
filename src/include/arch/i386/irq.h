#include <arch/i386/idt.h>
#include <stdbool.h>

#define IRQ_OFFSET 32

typedef void (irq_handler_t)(irq_regs_t*);

void irq_init();
void irq_register(uint8_t num, irq_handler_t* handler);
bool irq_is_happening();
irq_regs_t* irq_get_current_regs();
void nmi_init();
