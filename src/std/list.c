#include <list.h>
#include <mem/kmalloc.h>

#include <stdlib.h>
#include <string.h>

#define LIST_GROWTH 32

list_t* list_new() {
	list_t* list = (list_t*) kmalloc(sizeof(list_t));

	if ( list == NULL ) {
		return NULL;
	}

	list_init(list);

	return list;
}

void list_init(list_t* list) {
	memset(list, 0, sizeof(list_t));
}

void list_add(list_t* list, void* data) {
	if ( list->index + 1 > list->count ) {
		void** tmp = krealloc(list->data, list->count + LIST_GROWTH);

		if ( tmp == NULL ) {
			/* TODO: PANIC */
			return;
		}

		list->data = tmp;
	}

	list->data[list->index++] = data;
}

void* list_get(list_t* list, size_t index) {
	if ( index > list->index || list->data == NULL ) {
		/* TODO: PANIC */
		return NULL;
	}

	return list->data[index];
}

void list_rem(list_t* list, size_t index) {
	if ( index > list->index || list->data == NULL ) {
		/* TODO: PANIC */
		return;
	}

	memcpy(&list->data[index], &list->data[index + 1], list->index - index - 1);
	list->index--;
}

ssize_t list_find(list_t* list, void* data) {
	if ( list->data == NULL ) {
		return -1;
	}

	for ( size_t i = 0; i < list->index; i++ ) {
		if ( list->data[i] == data ) {
			return i;
		}
	}

	return -1;
}

void list_free(list_t* list, list_data_free_t* free_fn) {
	if ( list->data != NULL ) {
		if ( free_fn != NULL ) {
			for ( int i = 0; i < list->index; i++) {
				free_fn(list->data[i]);
			}
		}

		kfree(list->data);
	}
}