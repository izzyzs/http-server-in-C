#ifndef KEY_VALUE_LIST_H
#define KEY_VALUE_LIST_H

#define INIT_LIST_LEN 10

struct key_value_pair {
    char *key;
    char *value;
};

struct key_value_list {
    struct key_value_pair *items;
    int quantity;
    int capacity;
};

struct key_value_list *allocate_list(int length);
struct key_value_list *create_key_value_list();
void free_list_items(struct key_value_pair *items);
void delete_list(struct key_value_list *list); 
void add_to_list(struct key_value_list *list, char *key, char *value);
void lengthen_list(struct key_value_list *list);

#endif