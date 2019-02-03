#ifndef _FILE_H
#define _FILE_H

// #include <kernel/task.h>
#include <list.h>
#include <stdint.h>

// Constants
#define MAXFILEPATH 128

// Flags
#define O_RD    (1 << 0)
#define O_WRT   (1 << 1)
#define O_RDWRT (O_RD | O_WRT)
#define O_APD   (1 << 2)
#define O_STAT  (1 << 3)

// Empty struct defs
struct file;
typedef struct file file_t;

struct fs;
typedef struct fs fs_t;

struct file_ref;
typedef struct file_ref file_ref_t;

typedef uint8_t file_flags_t;

typedef size_t fid_t;
#define FID_INVALID (fid_t) (-1)

// File System implementation 
typedef file_ref_t* (fs_open_t)(fs_t* fs, char* path, file_flags_t flags);
typedef size_t (fs_read_t)(fs_t* fs, file_ref_t* file, char* buffer, size_t size);
typedef size_t (fs_write_t)(fs_t* fs, file_ref_t* file, char* buffer, size_t size);
typedef void (fs_close_t)(fs_t* fs, file_ref_t* file);
typedef size_t (fs_list_t)(fs_t* fs, char* path, file_ref_t** found);

// Full struct definitions
typedef struct {
	fs_open_t* open;
	fs_read_t* read;
	fs_write_t* write;
	fs_close_t* close;
	fs_list_t* list;
} fs_op_t;

struct fs {
	fs_op_t op;
	void* data;
};

struct file {
	fs_t* fs;
	char name[MAXFILEPATH];
	void* data;

	file_t* next;
	file_t* child;
};

// A task's reference to a file
typedef struct task_s task_t;
struct file_ref {
	fid_t fid;
	file_t* file;
	size_t index;
	task_t* task;
	file_flags_t flags;
};

// Table for fid -> file_ref pairs
typedef struct {
	size_t fid;
	file_ref_t* ref;
} file_ref_pair_t;

typedef struct {
	size_t max_fid;
	list_t pairs;
} file_ref_table_t;

// File functions
fid_t file_open(char* path, file_flags_t flags);
size_t file_read(fid_t fid, char* buffer, size_t size);
size_t file_write(fid_t fid, char* buffer, size_t size);
void file_close(fid_t fid);

// File System functions
fs_t* fs_init(fs_op_t* op);

// Private file functions
file_t* file_alloc(char* name, fs_t* fs, void* data);

// Allocate a file_ref
file_ref_t* file_ref_new(file_t* file, file_flags_t flags);

// Resolve a fid on a task
file_ref_t* file_ref_resolve(task_t* task, fid_t fid);
#endif