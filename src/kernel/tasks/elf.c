#include <kprint.h>
#include <stdlib.h>
#include <stdint.h>
#include <kernel/task.h>
#include <mem/slab.h>
#include <mem/slub.h>

#define ELF_MAGIC 0x7f454c46

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_ENDIAN_LITTLE 1
#define ELF_ENDIAN_BIG 2

#define ELF_VERSION 1

#define ELF_OSABI 0

#define ELF_ABIVERSION 0

#define ELF_TYPE_NONE 0x00
#define ELF_TYPE_REL 0x01
#define ELF_TYPE_EXEC 0x02
#define ELF_TYPE_DYN 0x03
#define ELF_TYPE_CORE 0x04
#define ELF_TYPE_LOOS 0xFE00
#define ELF_TYPE_HIOS 0xFEFF
#define ELF_TYPE_LOPROC 0xFF00
#define ELF_TYPE_HIPROC 0xFFFF

#define ELF_MACH_NONE 0x00
#define ELF_MACH_SPARC 0x02
#define ELF_MACH_X86 0x03
#define ELF_MACH_MIPS 0x08
#define ELF_MACH_POWERPC 0x14
#define ELF_MACH_S390 0x16
#define ELF_MACH_ARM 0x28
#define ELF_MACH_SUPERH 0x2A
#define ELF_MACH_IA64 0x32
#define ELF_MACH_X86_64 0x3E
#define ELF_MACH_AARCH64 0xB7
#define ELF_MACH_RISCV 0xF3

#define ELF_NIDENT	16

typedef struct {
	uint32_t magic;
	uint8_t class;
	uint8_t endian;
	uint8_t hdr_version;
	uint8_t os_abi;
	uint8_t os_abiversion;
	uint8_t pad[7];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
} __attribute__((packed)) elf_header_t;

typedef struct {
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} __attribute__((packed)) elf_header32_t;

typedef struct {
	fid_t file;
	task_t* target;

	uintptr_t entry;
} elf_t;

static
int elf_load_32(elf_t* elf) {
	elf_header32_t hdr32 = {};

	if ( file_read(elf->file, (void*) &hdr32, sizeof(hdr32)) != sizeof(hdr32) ) {
		return 1;
	}

	elf->entry = hdr32.entry;
	

	return 0;
}

static
int elf_load_64(elf_t* elf) {
	// not implemented
	return 1;
}

elf_t* elf_load(task_t* target, char* path) {
	elf_t* elf = mem_cache_alloc("elf cache");

	if ( elf == NULL ) {
		return NULL;
	}

	elf->file = file_open(path, O_RD);
	elf_header_t hdr = {};

	if ( file_read(elf->file, (void*) &hdr, sizeof(hdr)) != sizeof(hdr) ) {
		kprintf("elf: failed to read header\n");
		goto error;
	}

	if ( hdr.magic != ELF_MAGIC ) {
		kprintf("elf: magic incorrect\n");
		goto error;
	}

	if ( hdr.endian != ELF_ENDIAN_LITTLE ) {
		kprintf("elf: must be little endian\n");
		goto error;
	}

	if ( hdr.hdr_version != ELF_VERSION ) {
		kprintf("elf: version wrong\n");
		goto error;
	}

	if ( hdr.os_abi != ELF_OSABI ) {
		kprintf("elf: os_abi wrong\n");
		goto error;
	}

	if ( hdr.os_abiversion != ELF_ABIVERSION ) {
		kprintf("elf: hdr wrong\n");
		goto error;
	}

	if ( hdr.type != ELF_TYPE_EXEC ) {
		kprintf("elf: type wrong\n");
		goto error;
	}

	if ( hdr.machine != ELF_MACH_X86 ) {
		kprintf("elf: machine wrong\n");
		goto error;
	}

	if ( hdr.version != ELF_VERSION ) {
		kprintf("elf: hdr version wrong\n");
		goto error;
	}


	if ( hdr.class == ELF_CLASS_32 ) {
		if ( elf_load_32(elf) != 0 ) {
			goto error;
		}
	}
	else {
		if ( elf_load_64(elf) != 0 ) {
			goto error;
		}
	}

	return elf;

error:
	file_close(elf->file);
	mem_cache_dealloc("elf cache", elf);

	return NULL;

}

void elf_init() {
	(void) mem_cache_new("elf cache", sizeof(elf_t), 0, NULL, NULL);

	kprintf("elf: init done\n");
}
