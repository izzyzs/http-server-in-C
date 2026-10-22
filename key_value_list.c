#include "key_value_list.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct key_value_list *allocate_list(int length) {
    size_t size = length * sizeof(struct key_value_pair);
    struct key_value_pair *items = (struct key_value_pair*)malloc(size);
    if (items == NULL) {
        printf("Error: failed to allocate list items\n");
        return NULL;
    }
    struct key_value_list *list = (struct key_value_list*)malloc(sizeof(void*) + sizeof(int) + sizeof(int));
    if (list == NULL) {
        printf("Error: failed to allocate key value list\n");
        return NULL;
    }
    list->items = items;
    list->quantity = 0;
    list->capacity = length;
    return list;
}

struct key_value_list* create_key_value_list() {
    struct key_value_list *list = allocate_list(INIT_LIST_LEN);
    if (list == NULL) {
        printf("Error: failed to create list\n");
    }
    return list;
}

void free_list_items(struct key_value_pair *items) {
    free(items);
}

void delete_list(struct key_value_list *list) {
    free_list_items(list->items);
    free(list);
}

void add_to_list(struct key_value_list *list_pointer, char *key, char *value) {
    int space_remaining = list_pointer->capacity - list_pointer->quantity;
    if (space_remaining == 0) {
        lengthen_list(list_pointer);
    }
    struct key_value_pair *empty_value = list_pointer->items + list_pointer->quantity;
    empty_value->key = key;
    empty_value->value = value;
    list_pointer->quantity++;
}

void lengthen_list(struct key_value_list *list_pointer) {
    int quantity = list_pointer->quantity;
    int new_length = list_pointer->capacity * 2;
    struct key_value_pair *new_items = (struct key_value_pair*)malloc(sizeof(struct key_value_pair) * new_length);
    memcpy(list_pointer->items, new_items, new_length);
    free_list_items(list_pointer->items);
    list_pointer->items = new_items;
    list_pointer->capacity = new_length;
}
