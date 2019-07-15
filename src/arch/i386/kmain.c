#include <kernel/config.h>

#include <arch/i386/gdt.h>
#include <arch/i386/map.h>
#include <arch/i386/paging.h>

#ifdef BRACKOS_CONF_ACPI
#include <arch/i386/acpi.h>
#endif

#include <arch/i386/apic.h>
#include <arch/i386/pic.h>
#include <arch/i386/io.h>

#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/pit.h>
#include <kernel/clock.h>

#include <arch/i386/tss.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>

#include <kernel/pci.h>
#include <drivers/ps2.h>
#include <drivers/rs232.h>
#include <drivers/vga.h>
#include <drivers/rtl8139.h>

#include <kernel/vfs.h>

#include <mem/frame.h>
#include <mem/slab.h>
#include <mem/slub.h>
#include <mem/early.h>
#include <mem/mmap.h>
#include <mem/vasa.h>

#include <multiboot.h>
#include <kprint.h>
#include <stdint.h>
#include <string.h>

extern void* page_table_base;
unsigned long kernel_base = 0xC0000000;

void quick_create(char* path) {
    file_close(file_open(path, O_WRT));
    kprintf("created file %s\n", path);
}

void quick_list(char* path) {
    kprintf("ls: %s\n", path);
    size_t count = file_list(path, NULL);
    fid_t* list = (fid_t*) kmalloc(sizeof(size_t) * count);

    file_list(path, list);
    char name[MAXFILEPATH];
    memset(name, 0, MAXFILEPATH);

    for ( size_t i = 0; i < count; i++ ) {
        file_read(list[i], name, STAT_KEY('NAME'));
        kprintf("- %s\n", name);
    }
}

void file_test() {
    kprintf("doing file tests\n");

    quick_create("/test");
    quick_create("/asdf");
    quick_create("/test/test.txt");
    quick_create("/test/asdf.txt");
    quick_create("/asdf/test.txt");
    quick_create("/asdf/asdf");
    quick_create("/asdf/asdf/test.txt");

    quick_list("/");
    quick_list("/test");
    quick_list("/asdf");
    quick_list("/asdf/asdf");
}

void late_kernel_main() {
    kprintf("late main: Hello from late main!\n");
    vfs_init();             // Setup the file system

    pci_init();             // Setup the PCI
    rtl8139_init();         // Setup the RTL8139 drivers
    ps2_init();             // Setup PS/2 drivers

    file_test();

    kprintf("free phys mem: %m\n", frame_free_count());

    

    for ( ;; ) {}
}

// Load memory
void load_memory(multiboot_info_t* multiboot, unsigned long kernel_heap_start,
                                              unsigned long kernel_heap_size) {
    // Enable early stage kmalloc/memmap
    early_kmalloc_init((void*) kernel_heap_start, kernel_heap_size);
    memmap_to_frames(multiboot);
    vasa_init((void*) 0xD0000000, (uintptr_t) page_table_base - 0xD0000000);

    // Initiate late stage memory
    frame_init();       // Setup the frame allocator
    kmalloc_init();     // Setup late stage kmalloc
    vasa_switch();      // Switch the vasa into late stage mode
    kmem_swap();        // Switch to late stage memory
}

// Load the interrupt routines
void load_interrupts(bool apic_overide) {
    idt_init();         // Intialize the Interrupt Descriptor Table
    idt_load();         // Load the intiailized IDT

    kprintf("kmain: enabling the pic\n");
    pic_enable(0x20, 0x28);

#ifdef BRACKOS_CONF_ACPI
    bool acpi = acpi_init();

    if ( acpi && apic_supported() && !apic_overide ) {
        kprintf("kmain: enabling the apic\n");

        pic_disable();   // Disable the PIC
        lapic_enable();  // Enable the APIC

        ioapic_enable_irq(acpi_irq_remap(0), 0x20 + 0);
        ioapic_enable_irq(acpi_irq_remap(1), 0x20 + 1);
        ioapic_enable_irq(acpi_irq_remap(11), 0x20 + 11);
        ioapic_enable_irq(acpi_irq_remap(12), 0x20 + 12);

        kprintf("kmain: apic enabled: %d\n", lapic_is_enabled());

#ifdef BRACKOS_CONF_SMP
        smp_init();
#endif
    }
#endif

    irq_init();         // Setup Interrupt Requests
    exception_init();   // Setup CPU exceptions
    pit_init();         // Setup the Programmable Interrupt Timer
}

void kernel_main(uint32_t multiboot_magic,
                 multiboot_info_t* multiboot,
                 uintptr_t initial_pd,
                 uintptr_t kernel_heap_start,
                 size_t kernel_heap_size) {
    if ( multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC ) {
        // TODO - have an actual panic
        for ( ;; ) {}
    }

    gdt_init();             // Initialize the global descriptor table
    load_memory(multiboot, kernel_heap_start, kernel_heap_size); // Load memory
    rs232_init();           // Enable RS232 serial
    vga_init();             // Enable VGA output
    load_interrupts(false); // Load interrupts
    tss_init();             // Setup the task segment selector
    clock_init();           // Setup the clock subsystem

    // Setup tasks with the first being the late kernel
    task_init((uintptr_t) late_kernel_main);

    kprintf("kmain: enabling interrupts...\n");
    __asm__ volatile ("sti");

    for ( ;; ) {}
}
