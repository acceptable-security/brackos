#include <stdint.h>
#include <kernel/file.h>
#include <list.h>

typedef struct {
	char name[MAXFILEPATH];
	char* data;
	size_t len;

	file_t* child_root;
} vfs_node_t;

void vfs_init();