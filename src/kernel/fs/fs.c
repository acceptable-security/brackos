#include <kernel/file.h>
#include <stdlib.h>
#include <string.h>

fs_t* fs_init(fs_op_t* op) {
	fs_t* fs = (fs_t*) kmalloc(sizeof(fs_t));

	if ( fs == NULL ) {
		return NULL;
	}

	memset(fs, 0, sizeof(fs_t));
	memcpy(&fs->op, op, sizeof(fs_op_t));

	return fs;
}

