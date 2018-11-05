#include <stdint.h>

#define MAX_NAME_SZ 256

#define O_RD  (1 << 0)
#define O_WRT (1 << 1)
#define O_RDWRT (O_RD | O_WRT)
#define O_APD (1 << 2)

typedef int (fs_open_t)(char* path, uint32_t flags);
typedef void (fs_close_t)(int fd);
typedef int (fs_write_t)(int fd, char* data, uintptr_t count);
typedef int (fs_read_t)(int fd, char* data, uintptr_t count);

typedef struct {
	char fs_name[MAX_NAME_SZ];
	fs_open_t* open;
	fs_close_t* close;
	fs_write_t* write;
	fs_read_t* read;
	void* data;
} fs_t;

struct file_s;
typedef struct file_s file_t;

struct file_s {
	char name[MAX_NAME_SZ];
	uintptr_t flags;
	fs_t* fs;
};