#include <kernel/file.h>
#include <kernel/mount.h>
#include <kernel/scheduler.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>

file_t* file_alloc(char* name, fs_t* fs, void* data) {
	size_t len = strnlen(name, MAXFILEPATH) + 1;

	if ( len > MAXFILEPATH ) {
		// TODO - error messages
		kprintf("file: name too long\n");
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

	list_init(&file->stats.keys);
	list_init(&file->stats.vals);

	return file;
}

void file_add_friend(file_t* root, file_t* friend) {
	friend->next = root->next;
	root->next = friend;
}

void file_add_child(file_t* root, file_t* child) {
	if ( root->child == NULL ) {
		root->child = child;
	}
	else {
		child->next = root->child;
		root->child = child;
	}
}

size_t file_count_friends(file_t* file) {
	size_t count = 0;

	while ( file != NULL ) {
		count++;
		file = file->next;
	}

	return count;
}

void file_stat_set(file_t* file, stat_key_t key, void* data) {
	ssize_t index = list_find(&file->stats.keys, (void*) key);

	if ( index >= 0 ) {
		file->stats.vals.data[index] = data;
	}
	else {
		list_add(&file->stats.keys, (void*) key);
		list_add(&file->stats.vals, data);		
	}
}

void* file_stat_get(file_t* file, stat_key_t key) {
	ssize_t index = list_find(&file->stats.keys, (void*) key);

	if ( index < 0 ) {
		return NULL;
	}
	else {
		return file->stats.vals.data[index];
	}
}

fid_t file_open(char* path, file_flags_t flags) {
	char found_path[MAXFILEPATH];
	memset(found_path, 0, MAXFILEPATH);

	fs_t* fs = mount_find(path, found_path, NULL);

	if ( fs == NULL ) {
		kprintf("file: unable to find mount\n");
		return FID_INVALID;
	}

	file_ref_t* ref = fs->op.open(fs, &path[strlen(found_path) - 1], flags);

	if ( ref == NULL ) {
		kprintf("file: failed to get reference\n");
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

size_t file_list(char* path, fid_t* fids) {
	char found_path[MAXFILEPATH];
	memset(found_path, 0, MAXFILEPATH);
	
	fs_t* fs = mount_find(path, found_path, NULL);

	if ( fs == NULL ) {
		kprintf("file: unable to find mount\n");
		return FID_INVALID;
	}

	/* TODO: literally anything except this */
	size_t count = fs->op.list(fs, &path[strlen(found_path) - 1], NULL);
	file_ref_t** tmp = kmalloc(sizeof(file_ref_t*) * count);

	if ( tmp == NULL ) {
		return 0;
	}

	fs->op.list(fs, &path[strlen(found_path) - 1], tmp);

	for ( size_t i = 0; i < count; i++ ) {
		fids[i] = tmp[i]->fid;
	}

	kfree(tmp);

	return count;
}

void file_close(fid_t fid) {
	file_ref_t* ref = file_ref_resolve(current_task, fid);

	if ( ref == NULL ) {
		return;
	}

	fs_t* fs = ref->file->fs;
	return fs->op.close(fs, ref);
}