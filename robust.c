#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define INIT_LIST_LEN 10
typedef enum {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
} Method;


typedef enum {
    VERSION_OK,
    VERSION_INVALID_SYNTAX
} VersionStatus;
typedef struct {
    int major;
    int minor;
    VersionStatus status;
} Version;

typedef struct {
    char *key;
    char *value;
} KeyValuePair;

typedef struct {
    KeyValuePair *items;
    int quantity;
    int capacity;
} KeyValueList;

typedef struct {
    char *absolute_path;
    KeyValueList *query_values;
} URI;
typedef struct {
    Method method;
    URI uri;
    Version version;
} RequestLine;

void add_to_list(KeyValueList *list, char *key, char *value);

void lengthen_list(KeyValueList *list);

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n);
// ssize_t robust_write(int file_descriptor, void *user_buffer, size_t n);

Method parse_request_method(char *start_pointer);
URI parse_uri(char *start_pointer);
KeyValueList *parse_uri_query(char *start_pointer);
Version parse_http_version(char *start_pointer);

RequestLine parse_request_line(char *startp, int length);

void print_method(Method m);
void print_uri(URI u);
void print_http_version(Version v);
void print_request_line(RequestLine request);




int main(void) {
    int fd = open("./request.txt", O_RDONLY, 0);
    char buffer[1024];
    int read = robust_read(fd, buffer, 1024);
    printf("read: %d\n", read);
    printf("%s\n", strerror(errno));
    return 0;
}

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n) {

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

        printf("number_read: %zd\n", number_read);
        printf("buffer_pointer: %s\n\n", buffer_pointer);


        // process the newly read bytes
        char *char_1_pointer = start_of_unread, *char_2_pointer = start_of_unread+1, char_1, char_2;
        for (i = (start_of_unread-(char*)user_buffer); i < (buffer_pointer-start_of_unread); i++) {
            char_1 = *char_1_pointer;
            char_2 = *char_2_pointer;
            printf("processing bytes\n");
            printf("i = %ld\n", i);
            printf("\tchar_1 = %c\n\tchar_2 = %c\n", char_1, char_2);
             if (char_1 == '\r' && char_2 == '\n') {
                start_of_unread = char_2_pointer + 1;
                printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)start_of_line);
                RequestLine request = parse_request_line((char *)start_of_line, (int)(start_of_unread - start_of_line));
                print_request_line(request);
                // printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)user_buffer + (start_of_unread - (char*)user_buffer));
            } else {
                char_1_pointer += 1;
                char_2_pointer += 1;
            }

        }
        start_of_unread = buffer_pointer;
    }
    return (n - number_left); // returns the number of bytes filled bytes in the buffer in the case that there is nothing left to be read.
}

Method parse_request_method(char *start_pointer) {
    printf("parse_request_method\n");
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
    return GET;
}

URI parse_uri(char *start_pointer) {
    KeyValueList *query;
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
    URI uri = { path, query };
    return uri;
}

KeyValueList *allocate_list(int length) {
    size_t size = length * sizeof(KeyValuePair);
    KeyValuePair *items = (KeyValuePair*)malloc(size);
    KeyValueList *list = (KeyValueList*)malloc(sizeof(void*) + sizeof(int) + sizeof(int));
    list->items = items;
    list->quantity = 0;
    list->capacity = length;
    return list;
}

KeyValueList* create_list() {
    KeyValueList *list = allocate_list(INIT_LIST_LEN);
    return list;
}

void free_list_items(KeyValueList *list) {
    free(list->items);
}


void add_to_list(KeyValueList *list_pointer, char *key, char *value) {
    int space_remaining = list_pointer->capacity - list_pointer->quantity;
    if (space_remaining == 0) {
        lengthen_list(list_pointer);
    }
    KeyValuePair *empty_value = list_pointer->items + list_pointer->quantity;
    empty_value->key = key;
    empty_value->value = value;
    list_pointer->quantity++;
}

void lengthen_list(KeyValueList *list_pointer) {
    int quantity = list_pointer->quantity;
    int new_length = list_pointer->capacity * 2;
    KeyValuePair *new_items = (KeyValuePair*)malloc(sizeof(KeyValuePair) * new_length);
    memcpy(list_pointer->items, new_items, new_length);
    free_list_items(list_pointer);
    list_pointer->items = new_items;
    list_pointer->capacity = new_length;
}

KeyValueList *parse_uri_query(char *start_pointer) {
    KeyValueList *list_pointer = create_list();
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


Version parse_http_version(char *start_pointer) {
    Version version  = {0};
    int state;
    if ((state = (*start_pointer != 'H' || *(start_pointer + 1) != 'T' || *(start_pointer + 2) != 'T' || *(start_pointer + 3) != 'P' || *(start_pointer + 4) != '/'))) {
        version.status = VERSION_INVALID_SYNTAX;
        return version;
    }
    start_pointer += 5;
    version.major = *start_pointer++ - '0';
    version.minor = *++start_pointer - '0';
    version.status = VERSION_OK;
    return version;
}

void print_method(Method m) {
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

void print_http_version(Version v) {
    if (v.status == VERSION_INVALID_SYNTAX) {
        printf("ERROR: Invalid Syntax for HTTP version.");
    }
    printf("HTTP VERSION:\t%d.%d\n", v.major, v.minor);
}

void print_uri(URI u) {
    printf("URI\n---\n");
    printf("\tPath:\n\t-----\t\n\t\t%s\n", u.absolute_path);
    printf("\tQuery:\n\t------\n");
    for (int i = 0; i < u.query_values->quantity; i++) {
        KeyValuePair item = u.query_values->items[i];
        printf("\t\tkey:\t%s\t\tvalue:\t%s\n", item.key, item.value);
    }
}

RequestLine parse_request_line(char *start_pointer, int length) {
    Method method = parse_request_method(start_pointer);
    while (*start_pointer != ' ') {
        start_pointer++;
    }
    start_pointer++;
    URI uri = parse_uri(start_pointer);
    while (*start_pointer != ' ') {
        start_pointer++;
    }
    start_pointer++;
    Version version = parse_http_version(start_pointer);
    while (*start_pointer != '\n') {
        start_pointer++;
    }

    RequestLine r = {method, uri, version};
    return r;
}

void print_request_line(RequestLine request) {
    printf("Request line\n------------\n");
    print_method(request.method);
    print_http_version(request.version);
    print_uri(request.uri);
}