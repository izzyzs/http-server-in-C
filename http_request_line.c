#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_request_line.h"
#include "key_value_list.h"

struct http_request_line_state parse_request_line(struct http_request_line *request_line, struct http_request_line_state req_line_state, char **cursor_p, char *end_p) {
    int uri_state, version_state;
    enum http_method method = NONE;
    enum http_method_state method_state = HTTP_METHOD_START;
    struct http_uri uri;
    struct http_version version;
    struct http_request_line_state state = {HTTP_REQUEST_LINE_START, REQUEST_LINE_NO_ERR};

    // enum http_method_state parse_request_method(char *start_pointer, char* end_pointer, enum http_method *method, enum http_method_state method_state);
    method_state = parse_request_method(cursor_p, end_p, &method, method_state);
    if (method_state == HTTP_METHOD_PARSE_ERR) {
        state.status = ERROR;
        state.error = REQUEST_LINE_METHOD_ERR;
        return state;
    }
    else if (method_state == HTTP_METHOD_PARSED_AND_NOT_READ) {
        request_line->method = method;
        state.status = HTTP_REQUEST_LINE_METHOD_DONE;
        state.error = REQUEST_LINE_NO_ERR;
        return state;
    }

    uri_state = parse_uri(&uri, cursor_p);
    // if (uri_state > 0)
    //     return REQUEST_LINE_URI_ERR;

    // while (*cursor_p != ' ') {
    //     cursor_p++;
    // }
    // cursor_p++;

    // version_state = parse_http_version(&version, cursor_p);
    // if (uri_state > 0)
    //     return REQUEST_LINE_VERSION_ERR;

    // while (*cursor_p != '\n') {
    //     cursor_p++;
    // }

    // request_line->method = method;
    // request_line->uri = uri; 
    // request_line->version = version;
    // return REQUEST_LINE_SUCCESS;

    return state;
}

char *req_line_err_to_str(enum http_request_line_status status) {
    switch (status) {
        case REQUEST_LINE_METHOD_ERR: return "Method not properly formed\n These are the allowed methods: GET, QUERY, HEAD, OPTIONS, TRACE, PUT, DELETE, POST, PATCH, CONNECT";
        case REQUEST_LINE_URI_ERR: return "URI not properly formed";
        case REQUEST_LINE_VERSION_ERR: return "Not an HTTP version";
        default: return "";
    }
}

// static size_t get_method_length(enum http_method method) {
//     switch (method) {
//         case GET:
//             return (size_t)3;
//         case HEAD:
//             return (size_t)4;   
//         case POST:
//             return (size_t)4;
//         case PUT:
//             return (size_t)3;
//         case DELETE:
//             return (size_t)6;
//         case CONNECT:
//             return (size_t)7;
//         case OPTIONS:
//             return (size_t)7;
//         case TRACE:
//             return (size_t)5;
//     }
// }

enum http_method_state parse_request_method(char **cursor, char* end_pointer, enum http_method *method, enum http_method_state method_state) {
    
    if (method_state == HTTP_METHOD_START) {
        switch (**cursor) {
            case 'G':
                *method = GET;
                break;
            case 'H':
                *method = HEAD;
                break;
            case 'P':
                if (*(cursor + 1) == 'O')
                    *method = POST;
                else
                    *method = PUT;
                break;
            case 'D':
                *method = DELETE;
                break;
            case 'C':
                *method = CONNECT;
                break;
            case 'O':
                *method = OPTIONS;
                break;
            case 'T':
                *method = TRACE;
                break;
            default:
                return HTTP_METHOD_PARSE_ERR;
            }
        }
    if (*method < 1) // true if method is set to NONE -> 0 or ERROR -> -1
        return HTTP_METHOD_PARSE_ERR;

    while (*cursor != ' ') {
        if (cursor == end_pointer)
            return HTTP_METHOD_PARSED_AND_NOT_READ;
        cursor++;
    }
    cursor++;
    return HTTP_METHOD_PARSED_AND_READ;
}

int parse_uri(struct http_uri* uri, char *start_pointer, char *end_pointer) {
    struct key_value_list *query;
    int length;
    char *cursor = start_pointer;
    while (*cursor != ' ') {
        if (cursor != end_pointer) {
            return 
        }
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

char *http_method_to_str(enum http_method m) {
    if (m == GET)
        return "GET";
    if(m == HEAD)
        return "HEAD";
    if (m == POST) {
        return "POST";
    if (m == PUT)
        return "PUT";
    }
    if(m == DELETE)
        return "DELETE";
    if(m == CONNECT)
        return "CONNECT";
    if (m == OPTIONS)
        return "OPTIONS";
    if(m == TRACE)
        return "TRACE";
    return "";
}


void print_uri(struct http_uri u) {
    printf("HTTP URI\n---\n");
    printf("\tPath:\n\t-----\t\n\t\t%s\n", u.absolute_path);
    printf("\tQuery:\n\t------\n");
    for (int i = 0; i < u.query_values->quantity; i++) {
        struct key_value_pair item = u.query_values->items[i];
        printf("\t\tkey:\t%s\t\tvalue:\t%s\n", item.key, item.value);
    }
}

void print_http_version(struct http_version v) {
    if (v.status == VERSION_INVALID_SYNTAX) {
        printf("ERROR: Invalid Syntax for HTTP version.");
    }
    printf("HTTP VERSION:\t%d.%d\n", v.major, v.minor);
}

void print_request_line(struct http_request_line request) {
    printf("Request line\n------------\n");
    printf("HTTP METHOD:\t");
    printf("%s\n", http_method_to_str(request.method));
    print_http_version(request.version);
    print_uri(request.uri);
}