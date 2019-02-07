#include <kernel/file.h>
#include <kernel/vfs.h>
#include <kernel/mount.h>
#include <kprint.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

file_ref_t* vfs_open(fs_t* fs, char* path, file_flags_t flags) {
	char curr_file[MAXFILEPATH] = { 0 };

	file_t* prev_file = NULL;
	file_t* file = file_resolve((file_t*) fs->data, path, curr_file, &prev_file);

	if ( prev_file == NULL ) {
		kprintf("vfs: critical error no file\n");
		return NULL;
	}

	if ( file == NULL && (flags & O_WRT) == 0 ) {
		/* TODO: report no file found */
		kprintf("vfs: no file found\n");
		return NULL;
	}
	
	if ( file == NULL && (flags & O_WRT) == O_WRT ) {
		// Make sure we're top left in the curr_file
		char* tmp = curr_file;

		while ( *tmp ) {
			if ( *tmp == '/' ) {
				/* TODO: report no file found */
				kprintf("vfs: no file found\n");
				return NULL;
			}

			tmp++;
		}

		// Create node & file
		vfs_node_t* vfs = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
		MEMZERO(vfs);

		file = file_alloc(curr_file, fs, vfs);
		file_add_child(prev_file, file);
	}

	if ( ((flags & O_RD) == O_RD || (flags & O_WRT) == O_WRT) ) {
		file_ref_t* ref = file_ref_new(file, flags);
		return ref;
	}
	else if ( (flags & O_STAT) == O_STAT ) {
		// Open file for stat
		file_ref_t* ref = file_ref_new(file, flags);
		return ref;
	}

	return NULL;
}

static size_t vfs_read_stat_size(file_t* file, stat_key_t key) {
	switch ( key ) {
		case STAT_KEY('LIST'):
			return sizeof(stat_key_t) * file->stats.keys.index;
		case STAT_KEY('NAME'):
			return strlen(file->name) + 1;
		case STAT_KEY('SIZE'): 
			return sizeof(size_t);
		default:
			return 0;
	}
}

static size_t vfs_read_stat(file_t* file, char* buffer, stat_key_t key) {
	vfs_node_t* node = (vfs_node_t*) file->data;
	size_t size = vfs_read_stat_size(file, key);

	if ( buffer == NULL ) {
		return size;
	}
	else {
		void* data = NULL;

		switch ( key ) {
			case STAT_KEY('LIST'): {
				data = file->stats.keys.data;
				break;
			}

			case STAT_KEY('NAME'): {
				data = file->name;
				break;
			}

			case STAT_KEY('SIZE'): {
				data = &node->len;
				break;
			}
		}

		if ( data == NULL ) {
			return 0;
		}

		memcpy(buffer, data, size);

		return size;
	}

	return 0;
}

size_t vfs_read(fs_t* fs, file_ref_t* ref, char* buffer, ssize_t _size) {
	file_t* file = ref->file;
	vfs_node_t* node = (vfs_node_t*) file->data;

	if ( node == NULL ) {
		return 0;
	}

	if ( (ref->flags & O_STAT) == O_STAT ) {
		return vfs_read_stat(file, buffer, (stat_key_t) _size);
	}

	char* data = node->data;
	size_t len = node->len;

	size_t index = ref->index;
	size_t size = 0;

	if ( _size < 0 ) {
		if ( _size == RD_START ) {
			ref->index = 0;
			return index;
		}
		else if ( _size == RD_END ) {
			if ( buffer == NULL ) {
				ref->index = len;
				return len - index;
			}
			else {
				size = len - ref->index;
			}
		}
		else {
			 return 0;
		}
	}
	else {
		size = (size_t) _size;
	}

	size_t read_size = min(len - ref->index, size);

	if ( read_size > 0 ) {
		memcpy(buffer, &data[ref->index], read_size);		
	}

	ref->index += read_size;

	return read_size;
}

size_t vfs_write(fs_t* fs, file_ref_t* file, char* buffer, size_t size) {
	return 0;
}

void vfs_close(fs_t* fs, file_ref_t* file) {
	file_ref_close(file);
}

size_t vfs_list(fs_t* fs, char* path, file_ref_t** files) {
	char curr_file[MAXFILEPATH];
	memset(curr_file, 0, MAXFILEPATH);

	file_t* prev_file = NULL;
	file_t* file = file_resolve((file_t*) fs->data, path, curr_file, &prev_file);

	if ( file == NULL ) {
		/* TODO: report no file found */
		kprintf("vfs: failed to find file `%s`\n", path);
		return 0;
	}

	if ( files == NULL ) {
		return file_count_friends(file->child);
	}
	else {
		size_t index = 0;
		file_t* search = file->child;

		while ( search != NULL ) {		
			files[index] = file_ref_new(search, O_STAT);
			search = search->next;
			index++;
		}

		return index;
	}
}

static fs_op_t vfs_ops = {
	.open = vfs_open,
	.read = vfs_read,
	.write = vfs_write,
	.close = vfs_close,
	.list = vfs_list
};

void vfs_init() {
	fs_t* vfs = fs_init(&vfs_ops);

	vfs_node_t* node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	MEMZERO(node);

	vfs->data = file_alloc("", vfs, node);
	mount_root(vfs);

	kprintf("vfs: init complete\n");
}