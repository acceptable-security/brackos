#include <kernel/file.h>
#include <kernel/vfs.h>
#include <kernel/mount.h>
#include <kprint.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

file_t* global_root;

static file_t* vfs_find(fs_t* fs, char* path, char* curr_file) {
	vfs_node_t* vfs = (vfs_node_t*) fs->data;
	file_t* file = vfs->child_root;
	file_t* found = NULL;

	while ( vfs != NULL && *path ) {
		// Skip preceding /
		if ( *path == '/' ) path++;

		// Copy next file
		char* tmp = curr_file;
		while ( *path && *path != '/' ) *(tmp++) = *(path++);
		*tmp = 0;

		// Find curr_file in file friends
		file_t* search = file;

		while ( search != NULL ) {
			if ( strcmp(search->name, curr_file) == 0 ) {
				found = search;
				break;
			}

			search = search->next;
		}

		if ( found != NULL ) {
			break;
		}

		// Go down tree
		file = file->child;
	}

	return NULL;
}

file_ref_t* vfs_open(fs_t* fs, char* path, file_flags_t flags) {
	char curr_file[MAXFILEPATH] = { 0 };

	file_t* file = vfs_find(fs, path, curr_file);

	if ( file == NULL && (flags & O_WRT) == 0 ) {
		/* TODO: report no file found */
		return NULL;
	}
	else if ( file == NULL && (flags & O_WRT) == O_WRT ) {
		// Make sure we're top left in the curr_file
		char* tmp = curr_file;

		while ( *tmp ) {
			if ( *tmp == '/' ) {
				/* TODO: report no file found */
				return NULL;
			}

			tmp++;
		}

		// Create file
	}
	else if ( file != NULL && ((flags & O_RD) == O_RD || (flags & O_WRT) == O_WRT) ) {
		// Open file for RW
	}
	else if ( (flags & O_STAT) == O_STAT ) {
		// Open file for stat
	}


	return NULL;
}

size_t vfs_read(fs_t* fs, file_ref_t* file, char* buffer, size_t size) {
	return 0;
}

size_t vfs_write(fs_t* fs, file_ref_t* file, char* buffer, size_t size) {
	return 0;
}

void vfs_close(fs_t* fs, file_ref_t* file) {

}

size_t vfs_list(fs_t* fs, char* path, file_ref_t** files) {
	return 0;
}

static fs_op_t vfs_ops = {
	.open = vfs_open,
	.read = vfs_read,
	.write = vfs_write,
	.close = vfs_close,
	.list = vfs_list
};

void vfs_init() {
	global_root = file_alloc("", NULL, NULL);
	fs_t* vfs = fs_init(&vfs_ops);
	mount_root(vfs);

	kprintf("vfs: init complete\n");
}