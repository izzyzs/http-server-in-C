#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_request_line.h"
#include "key_value_list.h"

/*
enum http_request_line_state parse_request_line(struct http_request_line *request_line, enum http_request_line_state req_line_parser, char **cursor_p, char *end_p) {
    int uri_state, version_state;
    enum http_method method = NONE;
    enum http_method_state method_state = HTTP_METHOD_START;
    struct http_uri uri;
    struct http_version version;
    enum http_request_line_state state =HTTP_REQUEST_LINE_START;

    // enum http_method_state parse_request_method(char *start_pointer, char* end_pointer, enum http_method *method, enum http_method_state method_state);
    
    parse_request_method(cursor_p, end_p, &method, method_state);
    if (method_state == HTTP_METHOD_PARSE_ERR) {
        state.status = ERROR;
        state.error = HTTP_REQUEST_LINE_METHOD_ERR;
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
*/





int method_parsing_for_GET(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'G') {
        *method_state_p = READ_G_EXPECTING_E_IN_GET;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_G_EXPECTING_E_IN_GET && *method_state_p <= READ_T_EXPECTING_SPACE_AFTER_GET) {
        return 1;
    }
    return 0;
}


int method_parsing_for_HEAD(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'H') {
        *method_state_p = READ_H_EXPECTING_E_IN_HEAD;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_H_EXPECTING_E_IN_HEAD && *method_state_p <= READ_D_EXPECTING_SPACE_AFTER_HEAD) {
        return 1;
    }
    return 0;
}



int method_parsing_for_PUT_or_POST_or_PATCH(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'P') {
        *method_state_p = READ_P_EXPECTING_EITHER_O_OR_U_OR_A;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_P_EXPECTING_EITHER_O_OR_U_OR_A && *method_state_p <= READ_T_EXPECTING_SPACE_AFTER_PUT) {
        // note: only possible due to the the state enums that represent parsing
        // either POST or PUT exist continuously in the enum definition
        return 1;
    }
    return 0;
}

int method_parsing_for_POST(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == READ_P_EXPECTING_EITHER_O_OR_U_OR_A && *cursor == 'O') {
        *method_state_p = READ_O_EXPECTING_S_IN_POST;
        (*cursor)++;
        return 1;
    } else if (*method_state_p == READ_O_EXPECTING_S_IN_POST && *method_state_p == READ_T_EXPECTING_SPACE_AFTER_POST) {
        return 1;
    }
    return 0;
}

int method_parsing_for_PUT(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == READ_P_EXPECTING_EITHER_O_OR_U_OR_A && *cursor == 'U') {
        *method_state_p = READ_U_EXPECTING_T_IN_PUT;
        (*cursor)++;
        return 1;
    } else if (*method_state_p == READ_U_EXPECTING_T_IN_PUT && *method_state_p == READ_T_EXPECTING_SPACE_AFTER_PUT) {
        return 1;
    }
    return 0;
}

int method_parsing_for_PATCH(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == READ_P_EXPECTING_EITHER_O_OR_U_OR_A && *cursor == 'A') {
        *method_state_p = READ_A_EXPECTING_T_IN_PATCH;
        (*cursor)++;
        return 1;
    } else if (*method_state_p == READ_A_EXPECTING_T_IN_PATCH && *method_state_p == READ_H_EXPECTING_SPACE_AFTER_PATCH) {
        return 1;
    }
    return 0;
}


int method_parsing_for_DELETE(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'D') {
        *method_state_p = READ_D_EXPECTING_FIRST_E_IN_DELETE;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_D_EXPECTING_FIRST_E_IN_DELETE && *method_state_p <= READ_THIRD_E_EXPECTING_SPACE_AFTER_DELETE) {
        return 1;
    }
    return 0;
}

int method_parsing_for_CONNECT(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'C') {
        *method_state_p = READ_FIRST_C_EXPECTING_O_IN_CONNECT;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_FIRST_C_EXPECTING_O_IN_CONNECT && *method_state_p <= READ_SECOND_C_EXPECTING_T_IN_CONNECT) {
        return 1;
    }
    return 0;
}

int method_parsing_for_OPTIONS(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'O') {
        *method_state_p = READ_FIRST_O_EXPECTING_P_IN_OPTIONS;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_FIRST_O_EXPECTING_P_IN_OPTIONS && *method_state_p <= READ_S_EXPECTING_SPACE_AFTER_OPTIONS) {
        return 1;
    }
    return 0;
}

int method_parsing_for_TRACE(enum http_method_state *method_state_p, char **cursor) {
    if (*method_state_p == METHOD_STATE_START && **cursor == 'T') {
        *method_state_p = READ_T_EXPECTING_R_IN_TRACE;
        (*cursor)++;
        return 1;
    } else if (*method_state_p >= READ_T_EXPECTING_R_IN_TRACE && *method_state_p <= READ_E_EXPECTING_SPACE_AFTER_TRACE) {
        return 1;
    }
    return 0;
}

int method_parsing_not_done(enum http_method_state method_state) {
    return method_state != VALID_METHOD && method_state != INVALID_METHOD;
}


void parse_method_for_GET(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_G_EXPECTING_E_IN_GET && *cursor == 'E') {
            *method_state_p = READ_E_EXPECTING_T_IN_GET;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_E_EXPECTING_T_IN_GET && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_SPACE_AFTER_GET;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_SPACE_AFTER_GET && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = GET;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }    
}

void parse_method_for_HEAD(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_H_EXPECTING_E_IN_HEAD && *cursor == 'E') {
            *method_state_p = READ_E_EXPECTING_A_IN_HEAD;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_E_EXPECTING_A_IN_HEAD && *cursor == 'A') {
            *method_state_p = READ_A_EXPECTING_D_IN_HEAD;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_A_EXPECTING_D_IN_HEAD && *cursor == 'D') {
            *method_state_p = READ_D_EXPECTING_SPACE_AFTER_HEAD;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_D_EXPECTING_SPACE_AFTER_HEAD && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = HEAD;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}

void parse_method_for_PUT(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_U_EXPECTING_T_IN_PUT && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_SPACE_AFTER_PUT;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_SPACE_AFTER_PUT && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = PUT;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }    
}

void parse_method_for_POST(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_O_EXPECTING_S_IN_POST && *cursor == 'S') {
            *method_state_p = READ_S_EXPECTING_T_IN_POST;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_S_EXPECTING_T_IN_POST && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_SPACE_AFTER_POST;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_SPACE_AFTER_POST && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = POST;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}

void parse_method_for_PATCH(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_A_EXPECTING_T_IN_PATCH && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_C_IN_PATCH;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_C_IN_PATCH && *cursor == 'C') {
            *method_state_p = READ_C_EXPECTING_H_IN_PATCH;
            (*cursor)++;
            continue; 
        } else if (*method_state_p == READ_C_EXPECTING_H_IN_PATCH && *cursor == 'H') {
            *method_state_p = READ_H_EXPECTING_SPACE_AFTER_PATCH;
            (*cursor)++;
            continue;
        } 
        else if (*method_state_p == READ_H_EXPECTING_SPACE_AFTER_PATCH && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = PATCH;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}

void parse_method_for_DELETE(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_D_EXPECTING_FIRST_E_IN_DELETE && *cursor == 'E') {
            *method_state_p = READ_FIRST_E_EXPECTING_L_IN_DELETE;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_FIRST_E_EXPECTING_L_IN_DELETE && *cursor == 'L') {
            *method_state_p = READ_L_EXPECTING_SECOND_E_IN_DELETE;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_L_EXPECTING_SECOND_E_IN_DELETE && *cursor == 'E') {
            *method_state_p = READ_SECOND_E_EXPECTING_T_IN_DELETE;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_SECOND_E_EXPECTING_T_IN_DELETE && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_THIRD_E_IN_DELETE;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_THIRD_E_IN_DELETE && *cursor == 'E') {
            *method_state_p = READ_THIRD_E_EXPECTING_SPACE_AFTER_DELETE;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_THIRD_E_EXPECTING_SPACE_AFTER_DELETE && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = DELETE;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}
void parse_method_for_CONNECT(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_FIRST_C_EXPECTING_O_IN_CONNECT && *cursor == 'O') {
            *method_state_p = READ_O_EXPECTING_FIRST_N_IN_CONNECT;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_O_EXPECTING_FIRST_N_IN_CONNECT && *cursor == 'N') {
            *method_state_p = READ_FIRST_N_EXPECTING_SECOND_N_IN_CONNECT;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_FIRST_N_EXPECTING_SECOND_N_IN_CONNECT && *cursor == 'N') {
            *method_state_p = READ_SECOND_N_EXPECTING_E_IN_CONNECT;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_SECOND_N_EXPECTING_E_IN_CONNECT && *cursor == 'E') {
            *method_state_p = READ_E_EXPECTING_SECOND_C_IN_CONNECT;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_E_EXPECTING_SECOND_C_IN_CONNECT && *cursor == 'C') {
            *method_state_p = READ_SECOND_C_EXPECTING_T_IN_CONNECT;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_SECOND_C_EXPECTING_T_IN_CONNECT && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_SPACE_AFTER_CONNECT;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_SPACE_AFTER_CONNECT && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = CONNECT;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}
void parse_method_for_OPTIONS(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_FIRST_O_EXPECTING_P_IN_OPTIONS && *cursor == 'P') {
            *method_state_p = READ_P_EXPECTING_T_IN_OPTIONS;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_P_EXPECTING_T_IN_OPTIONS && *cursor == 'T') {
            *method_state_p = READ_T_EXPECTING_I_IN_OPTIONS;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_T_EXPECTING_I_IN_OPTIONS && *cursor == 'I') {
            *method_state_p = READ_I_EXPECTING_SECOND_O_IN_OPTIONS;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_I_EXPECTING_SECOND_O_IN_OPTIONS && *cursor == 'O') {
            *method_state_p = READ_SECOND_O_EXPECTING_N_IN_OPTIONS;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_SECOND_O_EXPECTING_N_IN_OPTIONS && *cursor == 'N') {
            *method_state_p = READ_N_EXPECTING_S_IN_OPTIONS;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_N_EXPECTING_S_IN_OPTIONS && *cursor == 'S') {
            *method_state_p = READ_S_EXPECTING_SPACE_AFTER_OPTIONS;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_S_EXPECTING_SPACE_AFTER_OPTIONS && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = OPTIONS;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}
void parse_method_for_TRACE(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end) {
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (*method_state_p == READ_T_EXPECTING_R_IN_TRACE && *cursor == 'R') {
            *method_state_p = READ_R_EXPECTING_A_IN_TRACE;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_R_EXPECTING_A_IN_TRACE && *cursor == 'A') {
            *method_state_p = READ_A_EXPECTING_C_IN_TRACE;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_A_EXPECTING_C_IN_TRACE && *cursor == 'C') {
            *method_state_p = READ_C_EXPECTING_E_IN_TRACE;
            (*cursor)++;
            continue;
        }
         else if (*method_state_p == READ_C_EXPECTING_E_IN_TRACE && *cursor == 'E') {
            *method_state_p = READ_E_EXPECTING_SPACE_AFTER_TRACE;
            (*cursor)++;
            continue;
        } else if (*method_state_p == READ_E_EXPECTING_SPACE_AFTER_TRACE && *cursor == ' ') {
            (*cursor)++;
            *method_state_p = VALID_METHOD;
            *http_method_p = TRACE;
            return;
        } else {
            *method_state_p = INVALID_METHOD;
            return;
        }
    }
}

void parse_request_method(struct http_req_line_parser *req_line_parser, char **cursor, char *end, enum http_method *method) {
    enum http_method_state *method_state_p = &(req_line_parser->method_state);
    enum http_method method;
    
    
    while (*cursor != end && method_parsing_not_done(*method_state_p)) {
        if (method_parsing_for_GET(method_state_p, cursor)) {
            parse_method_for_GET(&method, method_state_p, cursor, end);
        } else if (method_parsing_for_HEAD(method_state_p, cursor)) {
            parse_method_for_HEAD(&method, method_state_p, cursor, end);
        } else if (method_parsing_for_PUT_or_POST_or_PATCH(method_state_p, cursor)) {
            if (method_parsing_for_POST(method_state_p, cursor)) {
                parse_method_for_POST(&method, method_state_p, cursor, end);
            } else if (method_parsing_for_PUT(method_state_p, cursor)) {
                parse_method_for_PUT(&method, method_state_p, cursor, end);
            } else if (method_parsing_for_PATCH(method_state_p, cursor)) {
                parse_method_for_PATCH(&method, method_state_p, cursor, end);
            }
        } else if (method_parsing_for_DELETE(method_state_p, cursor)) {
            parse_method_for_DELETE(&method, method_state_p, cursor, end);
        } else if (method_parsing_for_CONNECT(method_state_p, cursor)) {
            parse_method_for_CONNECT(&method, method_state_p, cursor, end);
        } else if (method_parsing_for_OPTIONS(method_state_p, cursor)) {
            parse_method_for_OPTIONS(&method, method_state_p, cursor, end);
        } else if (method_parsing_for_TRACE(method_state_p, cursor)) {
            parse_method_for_TRACE(&method, method_state_p, cursor, end);
        }
    }
    }

// int parse_uri(struct http_uri* uri, char *start_pointer, char *end_pointer) {
//     struct key_value_list *query;
//     int length;
//     char *cursor = start_pointer;
//     while (*cursor != ' ') {
//         if (cursor != end_pointer) {
//             return 
//         }
//         if (*cursor == '?') {
//             query = parse_uri_query(++cursor);
//             --cursor;
//             break;
//         }
//         cursor++;
//     }
//     length = cursor - start_pointer;
//     char *path = (char*)malloc((length+1) * sizeof(char));
//     memcpy(path, start_pointer, length);
//     path[length] = '\0';
//     uri->absolute_path = path;
//     uri->query_values = query;
//     return 0;
// }

/**
struct key_value_list *parse_uri_query(char **cursor, char *end, struct http_req_line_parser *req_line_parser) {
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
 */

int parse_http_version(struct http_version *version, char *start_pointer, struct http_req_line_parser *req_line_parser) {
    int state;
    if ((state = (*start_pointer != 'H' || *(start_pointer + 1) != 'T' || *(start_pointer + 2) != 'T' || *(start_pointer + 3) != 'P' || *(start_pointer + 4) != '/'))) {
        // req_line_parser->version_state = VERSION_INVALID_SYNTAX;
        return -1;
    }
    start_pointer += 5;
    version->major = *start_pointer++ - '0';
    version->minor = *++start_pointer - '0';
    // req_line_parser->version_state = VERSION_OK;
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

// void print_http_version(struct http_version v) {
//     if (v.status == VERSION_INVALID_SYNTAX) {
//         printf("ERROR: Invalid Syntax for HTTP version.");
//     }
//     printf("HTTP VERSION:\t%d.%d\n", v.major, v.minor);
// }

void print_request_line(struct http_request_line request) {
    printf("Request line\n------------\n");
    printf("HTTP METHOD:\t");
    printf("%s\n", http_method_to_str(request.method));
    print_http_version(request.version);
    print_uri(request.uri);
}