#include <kernel/file.h>
#include <kernel/mount.h>
#include <kernel/scheduler.h>
#include <stdlib.h>
#include <string.h>

file_t* file_alloc(char* name, fs_t* fs, void* data) {
	size_t len = strnlen(name, MAXFILEPATH + 1) + 1;

	if ( len > MAXFILEPATH ) {
		// TODO - error messages
		return NULL;
	}

	file_t* file = (file_t*) kmalloc(sizeof(file_t));

	if ( file == NULL ) {
		return NULL;
	}

	MEMZERO(file);
	file->fs = fs;
	file->data = data;
	memcpy(&file->name, name, len * sizeof(char));

	return file;
}

fid_t file_open(char* path, file_flags_t flags) {
	char found_path[MAXFILEPATH];
	fs_t* fs = mount_find(path, found_path, NULL);

	if ( fs == NULL ) {
		return FID_INVALID;
	}

	file_ref_t* ref = fs->op.open(fs, &path[strlen(found_path)], flags);

	if ( ref == NULL ) {
		return FID_INVALID;
	}

	return ref->fid;
}

size_t file_read(fid_t fid, char* buffer, size_t size) {
	file_ref_t* ref = file_ref_resolve(current_task, fid);

	if ( ref == NULL ) {
		return 0;
	}

	fs_t* fs = ref->file->fs;
	return fs->op.read(fs, ref, buffer, size);
}

size_t file_write(fid_t fid, char* buffer, size_t size) {
	file_ref_t* ref = file_ref_resolve(current_task, fid);

	if ( ref == NULL ) {
		return 0;
	}

	fs_t* fs = ref->file->fs;
	return fs->op.write(fs, ref, buffer, size);
}

void file_close(fid_t fid) {
	file_ref_t* ref = file_ref_resolve(current_task, fid);

	if ( ref == NULL ) {
		return;
	}

	fs_t* fs = ref->file->fs;
	return fs->op.close(fs, ref);
}