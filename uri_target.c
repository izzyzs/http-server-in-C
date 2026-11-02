#include <stdlib.h>
#include <string.h>
#include "uri_target.h"
#include "key_value_list.h"
#include "serverror.h"

// requires checking the return for NULL
char *cpy_str(char *start_p, char *end_p) {
    int len = end_p - start_p;
    char *ret_text = (char *)malloc((len + 1) * sizeof(char));
    memcpy(ret_text, start_p, len);
    ret_text[len] = '\0';
    return ret_text;
}

int is_alpha(char c) {
    return (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

int is_digit(char c) {
    return c >= 0x30 && c <= 39;
}

int is_unreserved_char(char c) {
    return is_alpha(c) || is_digit(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

int parsing_for_asterisk_form(struct http_uri_target_parser *uri_parser_p, char **cursor) {
    if (uri_parser_p->state == UT_START && **cursor == '*') {
        uri_parser_p->state = UT_PARSE_FOR_ASTERISK_LOOK_FOR_SPACE;
        (*cursor)++;
        return 1;
    }
    return 0;
}

int parsing_for_origin_form(struct http_uri_target_parser *uri_parser_p, char **cursor) {
    if (uri_parser_p->state == UT_START && **cursor == '/') {
        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH;
        uri_parser_p->start_p = (*cursor);
        return 1;
    } else if (uri_parser_p->state >= UT_PARSE_FOR_ORIGIN_ABS_PATH && uri_parser_p->state <= UT_PARSE_FOR_ORIGIN_QUERY) {
        return 1;
    }
    return 0;
}

void parse_for_origin(struct request_line_uri_target *uri_target, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end) {
    char *val;
    uri_target->form = ORIGIN_FORM;

    while (*cursor != end) {
        switch (uri_parser_p->state) {
            case UT_PARSE_FOR_ORIGIN_ABS_PATH:
                if (**cursor == '?') {
                    val = cpy_str(uri_parser_p->start_p, *cursor);
                    if (val == NULL) {
                        print_server_error("malloc error in cpy_str");
                        uri_parser_p->state = INVALID_URI_TARGET;
                        return;
                    }
                    uri_target->uri_target.origin_form.absolute_path = val;
                    uri_parser_p->state = UT_PARSE_FOR_ORIGIN_QUERY;
                    uri_parser_p->start_p = cursor;
                } else if (**cursor == ' ') {
                    // handle no query
                    val = cpy_str(uri_parser_p->start_p, *cursor);
                    if (val == NULL) {
                        print_server_error("malloc error in cpy_str");
                        uri_parser_p->state = INVALID_URI_TARGET;
                        return;
                    }
                    uri_target->uri_target.origin_form.absolute_path = val;
                    uri_parser_p->state = VALID_URI_TARGET;
                    return;
                }
            break;
            case UT_PARSE_FOR_ORIGIN_QUERY:
                if (**cursor == ' ') {
                    val = cpy_str(uri_parser_p->start_p, *cursor);
                    if (val == NULL) {
                        print_server_error("malloc error in cpy_str");
                        uri_parser_p->state = INVALID_URI_TARGET;
                        return;
                    }
                    uri_target->uri_target.origin_form.query = val;
                    uri_parser_p->state = VALID_URI_TARGET;
                    uri_parser_p->start_p = cursor;
                    return;
                }
            break;
        }
        (*cursor)++;
    }
}

int main() {
    char *buffer = "/HELL";
    char **cursor = &buffer;
    struct http_uri_target_parser uri_target_parser;
    struct request_line_uri_target reql_uri_target;
    struct request_line_uri_target *reql_uri_target_p = &reql_uri_target;
    struct http_uri_target_parser *uri_parser_p = &uri_target_parser;

    uri_parser_p->state = UT_START;


    if (parsing_for_asterisk_form(uri_parser_p, cursor)) {
        printf("asterisk form\n");
        reql_uri_target_p->form = ASTERISK_FORM;
        uri_parser_p->state = VALID_URI_TARGET;
        // TODO: will have to check for method = OPTIONS and uri_target = ASTERISK match on return
        (*cursor)++;
    } else if (**cursor == '/') {
        printf("origin form\n");
    }

    return 0;
}

