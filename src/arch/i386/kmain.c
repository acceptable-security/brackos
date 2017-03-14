#include <multiboot.h>
#include <drivers/vga.h>
#include <stdint.h>

extern void gdt_init();

typedef struct multiboot_header multiboot_header_t;

void kernel_main(unsigned long multiboot_magic, multiboot_header_t* multiboot_header, unsigned long initial_pd) {
    if ( multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC ) {
        // TODO - have an actual panic
        for ( ;; ) {}
    }

	/* Initialize terminal interface */
	vga_init();

	/* Newline support is left as an exercise. */
	vga_writestring("Hello, kernel World!");

    gdt_init();
    for(;;){}
}
