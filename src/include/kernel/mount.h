#include <kernel/file.h>

typedef struct mount_node {
	char path[MAXFILEPATH];
	list_t children;
	fs_t* fs;
} mount_node_t;

mount_node_t* mount_node_new(fs_t* fs, char* path);
void mount_node_init(mount_node_t* node, fs_t* fs, char* path);
void mount_root(fs_t* fs);
fs_t* mount_find(char* path, char* found_path, mount_node_t** curr);
void mount(char* path, fs_t* fs);