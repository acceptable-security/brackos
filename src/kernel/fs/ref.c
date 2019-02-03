#include <kernel/file.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>

#include <stdlib.h>
#include <string.h>

file_ref_t* file_ref_new(file_t* file, file_flags_t flags) {
	// Setup reference
	file_ref_t* ref = (file_ref_t*) kmalloc(sizeof(file_ref_t));

	if ( ref == NULL ) {
		return NULL;
	}

	memset(ref, 0, sizeof(file_ref_t));

	ref->file = file;
	ref->index = 0;
	ref->task = current_task;
	ref->flags = flags; /* TODO: validate flags */

	// Assign fid and add to task list
	fid_t fid = current_task->files.max_fid++;

	file_ref_pair_t* pair = (file_ref_pair_t*) kmalloc(sizeof(file_ref_pair_t));

	if ( pair == NULL ) {
		kfree(ref);
		return NULL;
	}

	ref->fid = fid;
	pair->fid = fid;
	pair->ref = ref;

	list_add(&current_task->files.pairs, pair);

	return ref;
}

file_ref_t* file_ref_resolve(task_t* task, fid_t fid) {
	for ( size_t i = 0; i < current_task->files.pairs.index; i++ ) {
		file_ref_pair_t* pair = (file_ref_pair_t*) current_task->files.pairs.data[i];

		if ( pair->fid == fid ) {
			return pair->ref;
		}
	}

	return NULL;
}