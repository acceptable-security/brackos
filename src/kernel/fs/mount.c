#include <kernel/file.h>
#include <kernel/mount.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <list.h>
#include <kprint.h>

mount_node_t root_node;

mount_node_t* mount_node_new(fs_t* fs, char* path) {
	mount_node_t* node = (mount_node_t*) kmalloc(sizeof(mount_node_t));

	if ( node == NULL ) {
		return NULL;
	}

	mount_node_init(node, fs, path);
	return node;
}

void mount_node_init(mount_node_t* node, fs_t* fs, char* path) {
	memset(node, 0, sizeof(mount_node_t));

	if ( strlen(path) > MAXFILEPATH ) {
		/* TODO: PANIC */
		return;
	}

	strcpy(root_node.path, path);
	list_init(&root_node.children);
	node->fs = fs;
}

void mount_root(fs_t* fs) {
	mount_node_init(&root_node, fs, "/");
}

fs_t* mount_find(char* path, char* found_path, mount_node_t** curr) {
	mount_node_t* empty;

	if ( curr == NULL ) {
		curr = &empty;
	}

	*curr = &root_node;

	int dirty = 0;

	do {
		dirty = 0;
		strcpy(&found_path[strlen(found_path)], (*curr)->path);

		for ( size_t i = 0; i < (*curr)->children.index; i++ ) {
			mount_node_t* child = (*curr)->children.data[i];

			if ( strcmp(&path[strlen(found_path)], child->path) == 0 ) {
				(*curr) = child;
				dirty = 1;
				continue;
			}
		}
	} while(dirty);

	return (*curr)->fs;
}

void mount(char* path, fs_t* fs) {
	if ( strlen(path) > MAXFILEPATH ) {
		/* TODO: panic */
		return;
	}

	char found_path[MAXFILEPATH];
	mount_node_t* leaf_node = NULL;

	(void) mount_find(path, found_path, &leaf_node);
	/* TODO: use returned fs to validate path */

	char* adjusted_path = strdup(&path[strlen(found_path)]);
	mount_node_t* node = mount_node_new(fs, adjusted_path);

	if ( node == NULL ) {
		/* TODO: panic */
		return;
	}

	list_add(&leaf_node->children, node);
}