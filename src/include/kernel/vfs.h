#include <stdint.h>
#include <kernel/file.h>
#include <list.h>

typedef struct {
	char* data;
	size_t len;
} vfs_node_t;

void vfs_init();