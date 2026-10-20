#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

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

enum http_parser_state { START, REQUEST_LINE, FIELD_HEADERS, MESSAGE_BODY, COMPLETE, FAIL };

enum http_parser_event { COMPLETE_REQUEST_LINE, COMPLETE_FIELD_HEADERS, COMPLETE_MESSAGE_BODY, ERROR };


struct http_parser_state_machine { enum http_parser_state state; enum http_parser_event event };

// static struct http_parser_state_machine transtion(struct http_parser_state_machine) {
//     switch (s) {
        
//     }
// }

enum version_status { VERSION_OK, VERSION_INVALID_SYNTAX };
struct http_version {
    int major;
    int minor;
    enum version_status status;
};
struct http_uri {
    char *absolute_path;
    struct key_value_list *query_values;
};

enum http_method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

struct http_request_line {
    enum http_method method;
    struct http_uri uri;
    struct http_version version;
};

struct key_value_list *allocate_list(int length);
struct key_value_list *create_key_value_list();
void add_to_list(struct key_value_list *list, char *key, char *value);
void lengthen_list(struct key_value_list *list);
void free_list_items(struct key_value_pair *items);


ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n);
// ssize_t robust_write(int file_descriptor, void *user_buffer, size_t n);

enum http_request_line_status { REQUEST_LINE_METHOD_ERR, REQUEST_LINE_URI_ERR, REQUEST_LINE_VERSION_ERR, REQUEST_LINE_SUCCESS };

enum http_request_line_status parse_request_line(struct http_request_line *request_line, char *start_pointer, int length);
enum http_method parse_request_method(char *start_pointer);
int parse_uri(struct http_uri* uri, char *start_pointer);
struct key_value_list *parse_uri_query(char *start_pointer);
int parse_http_version(struct http_version *version, char *start_pointer);

struct key_value_list *parse_field_lines(char *start_pointer);



void print_method(enum http_method m);
void print_uri(struct http_uri u);
void print_http_version(struct http_version v);
void print_request_line(struct http_request_line request);




int main(void) {
    int fd = open("./request.txt", O_RDONLY, 0);
    char buffer[1024];
    int read = robust_read(fd, buffer, 1024);
    printf("read: %d\n", read);
    printf("%s\n", strerror(errno));
    return 0;
}

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n) {
    struct http_request_line request_line = {0};
    enum http_request_line_status request_line_status;
    struct key_value_list *field_lines = create_key_value_list();

    ssize_t number_left = n;
    ssize_t number_read;
    char *buffer_pointer = user_buffer;
    char *start_of_unread = user_buffer; // start of the unread portion of the application buffer
    char *start_of_line = user_buffer; // start of the last read line

    while (number_left > 0) {
        if ((number_read = read(file_descriptor, buffer_pointer, number_left)) < 0) { // meaning if an error has occured while reading i.e., read() returns -1
            if (errno == EINTR) // check to see if the read system call has been blocked by an interrupt
                number_read = 0;
            else
                return -1; // errno has been set by read and there's some other reason for the error
        }
        else if (number_read == 0) // ran into an EOF
            break; // don't call it again;

        long i;
        number_left -= number_read; // subtract the number of bytes successfully read from the number of bytes left to be read
        buffer_pointer += number_read; // increment the buffer pointer the number_read amount of bytes.


        // process the newly read bytes
        char *char_1_pointer = start_of_unread, *char_2_pointer = start_of_unread+1, char_1, char_2;
        for (i = (start_of_unread-(char*)user_buffer); i < (buffer_pointer-start_of_unread); i++) {
            char_1 = *char_1_pointer;
            char_2 = *char_2_pointer;
            // printf("processing bytes\n");
            // printf("i = %ld\n", i);
            // printf("\tchar_1 = %c\n\tchar_2 = %c\n", char_1, char_2);
             if (char_1 == '\r' && char_2 == '\n') {
                /*
                    if parser.state = 
                */
                start_of_unread = char_2_pointer + 1;
                // printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)start_of_line);
                request_line_status = parse_request_line(&request_line, (char *)start_of_line, (int)(start_of_unread - start_of_line));
            } else {
                char_1_pointer += 1;
                char_2_pointer += 1;
            }
            
        }
        start_of_unread = buffer_pointer;
    }
    print_request_line(request_line);
    return (n - number_left); // returns the number of bytes filled bytes in the buffer in the case that there is nothing left to be read.
}

enum http_method parse_request_method(char *start_pointer) {
    if (*start_pointer == 'G') {
        return GET;
    }
    if(*start_pointer == 'H') {
        return HEAD;
    }
    if (*start_pointer == 'P') {
        if (*(start_pointer + 1) == 'O') {
            return POST;
        } else {
            return PUT;
        }
    }
    if(*start_pointer == 'D') {
        return DELETE;
    }
    if(*start_pointer == 'C') {
        return CONNECT;
    }
    if(*start_pointer == 'O') {
        return OPTIONS;
    }
    if(*start_pointer == 'T') {
        return TRACE;
    }
    return -1;
}

int parse_uri(struct http_uri* uri, char *start_pointer) {
    struct key_value_list *query;
    int length;
    char *cursor = start_pointer;
    while (*cursor != ' ') {
        if (*cursor == '?') {
            query = parse_uri_query(++cursor);
            --cursor;
            break;
        }
        cursor++;
    }
    length = cursor - start_pointer;
    char *path = (char*)malloc((length+1) * sizeof(char));
    memcpy(path, start_pointer, length);
    path[length] = '\0';
    uri->absolute_path = path;
    uri->query_values = query;
    return 0;
}

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

struct key_value_list *parse_uri_query(char *start_pointer) {
    struct key_value_list *list_pointer = create_key_value_list();
    int token_length = 0;
    char *slow_cursor = start_pointer, *fast_cursor = start_pointer;
    char  *text_pointer, *key = NULL, *value = NULL;
    while (*fast_cursor != '#' && *fast_cursor != ' ') {
        fast_cursor++;
        if (*fast_cursor == '=') {
            token_length = fast_cursor-slow_cursor;
            text_pointer = (char*)malloc((token_length+1) * sizeof(char));
            memcpy(text_pointer, slow_cursor, token_length);
            text_pointer[token_length] = '\0';
            key = text_pointer;
            slow_cursor = ++fast_cursor;
        } else if (*fast_cursor == '&') {
            token_length = fast_cursor-slow_cursor;
            text_pointer = (char*)malloc((token_length+1) * sizeof(char));
            memcpy(text_pointer, slow_cursor, token_length);
            text_pointer[token_length] = '\0';
            value = text_pointer;
            slow_cursor = ++fast_cursor;
        }
        if(key && value) {
            add_to_list(list_pointer, key, value);
            key = NULL;
            value = NULL;
        }
    }
    token_length = fast_cursor-slow_cursor;
    text_pointer = (char*)malloc((token_length+1) * sizeof(char));
    memcpy(text_pointer, slow_cursor, token_length);
    text_pointer[token_length] = '\0';
    value = text_pointer;
    if(key && value) {
            add_to_list(list_pointer, key, value);
        }
    return list_pointer;
}


int parse_http_version(struct http_version *version, char *start_pointer) {
    int state;
    if ((state = (*start_pointer != 'H' || *(start_pointer + 1) != 'T' || *(start_pointer + 2) != 'T' || *(start_pointer + 3) != 'P' || *(start_pointer + 4) != '/'))) {
        version->status = VERSION_INVALID_SYNTAX;
        return -1;
    }
    start_pointer += 5;
    version->major = *start_pointer++ - '0';
    version->minor = *++start_pointer - '0';
    version->status = VERSION_OK;
    return 0;
}

void print_method(enum http_method m) {
    printf("HTTP METHOD:\t");
    if (m == GET)
        printf("GET\n");
    if(m == HEAD)
        printf("HEAD\n");
    if (m == POST) {
        printf("POST\n");
    if (m == PUT)
        printf("PUT\n");
    }
    if(m == DELETE)
        printf("DELETE\n");
    if(m == CONNECT)
        printf("CONNECT\n");
    if (m == OPTIONS)
        printf("OPTIONS\n");
    if(m == TRACE)
        printf("TRACE\n");
}

void print_http_version(struct http_version v) {
    if (v.status == VERSION_INVALID_SYNTAX) {
        printf("ERROR: Invalid Syntax for HTTP version.");
    }
    printf("HTTP VERSION:\t%d.%d\n", v.major, v.minor);
}

void print_uri(struct http_uri u) {
    printf("struct http_uri\n---\n");
    printf("\tPath:\n\t-----\t\n\t\t%s\n", u.absolute_path);
    printf("\tQuery:\n\t------\n");
    for (int i = 0; i < u.query_values->quantity; i++) {
        struct key_value_pair item = u.query_values->items[i];
        printf("\t\tkey:\t%s\t\tvalue:\t%s\n", item.key, item.value);
    }
}

// struct http_request_line parse_request_line(char *start_pointer, int length) {
enum http_request_line_status parse_request_line(struct http_request_line *request_line, char *start_pointer, int length) {
    int uri_state, version_state;
    enum http_method method;
    struct http_uri uri;
    struct http_version version;

    method = parse_request_method(start_pointer);
    if (method > 0)
        return REQUEST_LINE_METHOD_ERR;

    while (*start_pointer != ' ') {
        start_pointer++;
    }
    start_pointer++;

    uri_state = parse_uri(&uri, start_pointer);
    if (uri_state > 0)
        return REQUEST_LINE_URI_ERR;

    while (*start_pointer != ' ') {
        start_pointer++;
    }
    start_pointer++;

    version_state = parse_http_version(&version, start_pointer);
    if (uri_state > 0)
        return REQUEST_LINE_VERSION_ERR;

    while (*start_pointer != '\n') {
        start_pointer++;
    }

    request_line->method = method;
    request_line->uri = uri; 
    request_line->version = version;
    return REQUEST_LINE_SUCCESS;
}

void print_request_line(struct http_request_line request) {
    printf("Request line\n------------\n");
    print_method(request.method);
    print_http_version(request.version);
    print_uri(request.uri);
}