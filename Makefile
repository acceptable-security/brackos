ARCH = i386

# Programs to use for compilation
ASM = nasm
CC = i386-elf-gcc
LD = i386-elf-gcc

#FLAGS
DEFAULT_CFLAGS = -Wall -Werror -O -fomit-frame-pointer -ffreestanding  -finline-functions -c -g -std=c11
ASMFLAGS = -f elf
CFLAGS = $(DEFAULT_CFLAGS) -m32 -Isrc/include/ -march=i386
LDFLAGS = -m32 -ffreestanding -O2 -nostdlib -g

# The necessary source files
C_SOURCES = $(shell find src -name '*.c')
ASM_SOURCES = $(shell find src -iname '*.asm')

# Their complementary object files
C_OBJECTS = $(subst src, build, $(C_SOURCES:.c=.o))
ASM_OBJECTS = $(subst src, build, $(ASM_SOURCES:.asm=.o))

kernel: $(C_OBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -T src/arch/$(ARCH)/linker.ld -o kernel.bin $(ASM_OBJECTS) $(C_OBJECTS)

$(C_OBJECTS):
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(subst build, src, $(subst .o,.c,$@)) -o $@

$(ASM_OBJECTS):
	mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(subst build, src, $(subst .o,.asm,$@)) -o $@

clean:
	rm -f $(C_OBJECTS) $(ASM_OBJECTS) kernel.bin brackos.iso
	rm -rf isodir

image:
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o brackos.iso isodir

run:
	qemu-system-x86_64 -hda brackos.iso \
					   -d guest_errors \
					   -no-reboot \
					   -serial stdio \
					   -smp 2 \
					   -net nic,model=rtl8139

.PHONY: clean kernel run
