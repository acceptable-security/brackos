#ifndef _LIST_H
#define _LIST_H

#include <stdint.h>

typedef struct {
	size_t index;
	size_t count;
	void** data;
} list_t;


typedef void (list_data_free_t)(void* data);

list_t* list_new();
void list_init(list_t* list);
void list_add(list_t* list, void* data);
void* list_get(list_t* list, size_t index);
void list_rem(list_t* list, size_t index);
ssize_t list_find(list_t* list, void* data);
void list_free(list_t* list, list_data_free_t* free_fn);

#endif