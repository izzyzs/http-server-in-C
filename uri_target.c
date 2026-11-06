#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uri_target.h"
#include "key_value_list.h"
#include "serverror.h"


void handle_test_list(const char **uris, size_t list_len);
char *parser_state(struct http_uri_target_parser parser);
char *cpy_str(char *start_p, char *end_p);

int is_alpha(char c);
int is_digit(char c);
int is_unreserved_char(char c);
int is_hex_digit(char c);
int is_subdelimiter(char c);
int is_pchar(char c);

int parsing_for_asterisk_form(struct http_uri_target_parser *uri_parser_p, char **cursor);
int parsing_for_origin_form(struct http_uri_target_parser *uri_parser_p, char **cursor);
void init_for_origin_target(struct request_line_uri_target *uri_target_p);
void parse_for_origin(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end);    
int parsing_for_authority_form(struct http_uri_target_parser *uri_parser_p, char **cursor);
void parse_for_authority(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end);    
void test_origin_parser();

void print_uri(struct request_line_uri_target *uri_target);

int main() {
    
    
    return 0;
}


int parsing_for_authority_form(struct http_uri_target_parser *uri_parser_p, char **cursor);
void parse_for_authority(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end);    

// the uri strings were produced by ChatGPT to help with testing
void test_origin_parser() {

    const char *valid_uris[] = {
        // Basic paths
        "/ HTTP/1.1",
        "/index.html HTTP/1.1",
        "/hello HTTP/1.1",
        "/users/123 HTTP/1.1",
        "/api/v1/users HTTP/1.1",

        // Path structure
        "// HTTP/1.1",
        "/a//b HTTP/1.1",
        "/a/b/ HTTP/1.1",
        "/./ HTTP/1.1",
        "/../ HTTP/1.1",
        "/a/../b HTTP/1.1",

        // Unreserved characters
        "/abc-._~123 HTTP/1.1",
        "/hello_world HTTP/1.1",
        "/user~name HTTP/1.1",

        // Sub-delimiters
        "/hello!world HTTP/1.1",
        "/price$100 HTTP/1.1",
        "/a&b HTTP/1.1",
        "/it's HTTP/1.1",
        "/foo(bar) HTTP/1.1",
        "/a*b+c,d HTTP/1.1",
        "/a;b=c HTTP/1.1",

        // Other permitted path characters
        "/user:123 HTTP/1.1",
        "/user@domain HTTP/1.1",
        "/a:b@c HTTP/1.1",

        // Percent-encoding
        "/hello%20world HTTP/1.1",
        "/%2F HTTP/1.1",
        "/%2f HTTP/1.1",
        "/%41%42%43 HTTP/1.1",
        "/a%3Ab HTTP/1.1",
        "/%25 HTTP/1.1",
        "/%00 HTTP/1.1",

        // Queries
        "/?x=1 HTTP/1.1",
        "/?x=1&y=2 HTTP/1.1",
        "/search?q=hello%20world HTTP/1.1",
        "/path? HTTP/1.1",
        "/path?foo/bar HTTP/1.1",
        "/path?foo?bar HTTP/1.1",
        "/path?x=%26%3D HTTP/1.1",
        "/path?redirect=/a/b?x=1 HTTP/1.1",

        // Complex combinations
        "/api/v1/users/123?active=true&sort=asc HTTP/1.1",
        "/a%2Fb/c%20d?x=%25&y=hello HTTP/1.1",
        "/foo;bar=baz?x=a+b&y=c%2Fd HTTP/1.1"
    };

    const char *invalid_uris[] = {
    //     "hello HTTP/1.1",
    //     "index.html HTTP/1.1",
    //     "?x=1 HTTP/1.1",

        // Invalid percent-encoding
        "/hello% HTTP/1.1",
        "/hello%2 HTTP/1.1",
        "/hello%GG HTTP/1.1",
        "/hello%2G HTTP/1.1",
        "/hello%%20 HTTP/1.1",

        // Forbidden characters
        // "/hello<world HTTP/1.1",
        // "/hello>world HTTP/1.1",
        // "/hello\"world HTTP/1.1",
        // "/hello\\world HTTP/1.1",
        // "/hello^world HTTP/1.1",
        // "/hello|world HTTP/1.1",
        // "/hello`world HTTP/1.1",
        // "/hello{world HTTP/1.1",
        // "/hello}world HTTP/1.1",
        
        // Fragment identifiers are not allowed
        // "/hello#fragment HTTP/1.1",
        // "/path?x=1#section HTTP/1.1",

        // Empty request-target
        // " HTTP/1.1"
    };
    size_t valid_len = sizeof(valid_uris)/sizeof(valid_uris[0]);
    size_t invalid_len = sizeof(invalid_uris)/sizeof(invalid_uris[0]);
    printf("::::::::::\n");
    printf("VALID URIS\n");
    printf("::::::::::\n");
    handle_test_list(valid_uris, valid_len);
    printf("::::::::::::\n");
    printf("INVALID URIS\n");
    printf("::::::::::::\n");;
    handle_test_list(invalid_uris, invalid_len);
}

void handle_test_list(const char **uris, size_t list_len) {
    for (int i = 0; i < list_len; i++) {
        struct http_uri_target_parser uri_target_parser;
        struct request_line_uri_target reql_uri_target;
        struct http_uri_target_parser *uri_parser_p = &uri_target_parser;
        struct request_line_uri_target *reql_uri_target_p = &reql_uri_target;
        
        printf("attempting to parse %s\n", uris[i]);
        char *buffer = (char *)uris[i];
        size_t end_i = strlen(buffer) - 1;
        char *end = buffer + end_i;
        char **cursor = &buffer;

        uri_parser_p->state = UT_START;
        if (parsing_for_asterisk_form(uri_parser_p, cursor)) {
            printf("asterisk form\n");
            reql_uri_target_p->form = ASTERISK_FORM;
            uri_parser_p->state = VALID_URI_TARGET;
            // TODO: will have to check for method = OPTIONS and uri_target = ASTERISK match on return
            (*cursor)++;
            
        } else if (parsing_for_origin_form(uri_parser_p, cursor)) {
            parse_for_origin(reql_uri_target_p, uri_parser_p, cursor, end);
        }

        print_uri(reql_uri_target_p);
        printf("DEBUG: parser state %s\n\n", parser_state(*uri_parser_p));
    }
}

char *parser_state(struct http_uri_target_parser parser) {
    switch (parser.state) {
        case UT_START:
            return "UT_START";
        case UT_PARSE_FOR_ASTERISK_LOOK_FOR_SPACE:
            return "UT_PARSE_FOR_ASTERISK_LOOK_FOR_SPACE";
        case UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL:
            return "UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL";
        case UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1:
            return "UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1";
        case UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2:
            return "UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2";
        case UT_PARSE_FOR_ORIGIN_QUERY:
            return "UT_PARSE_FOR_ORIGIN_QUERY";
        case VALID_URI_TARGET:
            return "VALID_URI_TARGET";
        case INVALID_URI_TARGET:
            return "INVALID_URI_TARGET";
    }
}




void print_uri(struct request_line_uri_target *uri_target) {
    switch (uri_target->form) {
        case ORIGIN_FORM:
            printf("ORIGIN FORM\n");
            printf("absolute path: %s\t", uri_target->uri_target.origin_form.absolute_path);
            printf("strlen: %lu\n", strlen(uri_target->uri_target.origin_form.absolute_path));
            if (uri_target->uri_target.origin_form.query != NULL) {
                printf("query: %s\t\t", uri_target->uri_target.origin_form.query);
                printf("strlen: %lu\n", strlen(uri_target->uri_target.origin_form.query));
            }
        return;
        case ASTERISK_FORM:
            printf("asterisk form: *");
        return;
        case ABSOLUTE_FORM:

        return;
        case AUTHORITY_FORM:

        return;
        default:
            printf("Not printable");
        return;
    }
}


// requires checking the return for NULL
char *cpy_str(char *start_p, char *end_p) {
    int len = end_p - start_p;
    char *ret_text = (char *)malloc((len + 1) * sizeof(char));
    memcpy(ret_text, start_p, len);
    ret_text[len] = '\0';
    return ret_text;
}


void init_for_origin_target(struct request_line_uri_target *uri_target_p) {
    uri_target_p->uri_target.origin_form.absolute_path = "";
    uri_target_p->uri_target.origin_form.query = NULL;
}

int is_alpha(char c) {
    return (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

int is_digit(char c) {
    return c >= 0x30 && c <= 0x39;
}

int is_unreserved_char(char c) {
    return is_alpha(c) || is_digit(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

int is_hex_digit(char c) {
    return is_digit(c) || c >= 0x41 && c <= 0x46 || c >= 0x61 && c <= 0x66;
}


int is_subdelimiter(char c) {
    return c == 0x21 || c == 0x24 || c == 0x26 || c >= 0x27 && c <= 0x2c || c == 0x3b || c == 0x3d;
}

int is_pchar(char c) {
    return is_unreserved_char(c) || is_subdelimiter(c) || c == ':' || c == '@';
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
        uri_parser_p->start_p = (*cursor);
        (*cursor)++;
        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL;
        return 1;
    } else if (uri_parser_p->state >= UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL && uri_parser_p->state <= UT_PARSE_FOR_ORIGIN_QUERY) {
        return 1;
    }
    return 0;
}

void parse_for_origin(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end) {
    char *val;
    init_for_origin_target(uri_target_p);
    
    while (*cursor != end) {
        if (**cursor == '#') {
            uri_parser_p->state = INVALID_URI_TARGET;
            uri_target_p->form = URI_FORM_NOT_SET;
            return;
        } else if (uri_parser_p->state >= UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL && uri_parser_p->state <=UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2) {
                if (**cursor == '?') {
                    val = cpy_str(uri_parser_p->start_p, *cursor);
                    if (val == NULL) {
                        print_server_error("malloc error in cpy_str");
                        uri_parser_p->state = INVALID_URI_TARGET;
                        return;
                    }
                    uri_target_p->uri_target.origin_form.absolute_path = val;
                    uri_parser_p->state = UT_PARSE_FOR_ORIGIN_QUERY;
                    uri_parser_p->start_p = *cursor;
                } else if (**cursor == ' ') {
                    // handle no query
                    if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL) {
                        val = cpy_str(uri_parser_p->start_p, *cursor);
                        if (val == NULL) {
                            print_server_error("malloc error in cpy_str");
                            uri_parser_p->state = INVALID_URI_TARGET;
                            return;
                        }
                        uri_target_p->uri_target.origin_form.absolute_path = val;
                        uri_target_p->form = ORIGIN_FORM;
                        uri_parser_p->state = VALID_URI_TARGET;
                    } else {
                        uri_parser_p->state = INVALID_URI_TARGET;
                        uri_target_p->form = URI_FORM_NOT_SET;
                    }
                    return;
                } else if (**cursor == '%') {
                    if (uri_parser_p->state != UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL) {
                        uri_parser_p->state = INVALID_URI_TARGET;
                        uri_target_p->form = URI_FORM_NOT_SET;
                        return;
                    }
                    uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1;
                } else if (is_pchar(**cursor) || **cursor == '/') {
                    if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1 || uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2) {
                        if (!is_hex_digit(**cursor)) {
                            uri_parser_p->state = INVALID_URI_TARGET;
                            uri_target_p->form = URI_FORM_NOT_SET;
                            return;
                        }
                        if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1)
                        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2;
                        else
                        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL;
                    }
                } else {
                    uri_parser_p->state = INVALID_URI_TARGET;
                    uri_target_p->form = URI_FORM_NOT_SET;
                    return;
                }
        } else if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_QUERY) {
            if (**cursor == ' ') {
                val = cpy_str(uri_parser_p->start_p, *cursor);
                if (val == NULL) {
                    print_server_error("malloc error in cpy_str");
                    uri_parser_p->state = INVALID_URI_TARGET;
                    return;
                }
                uri_target_p->uri_target.origin_form.query = val;
                uri_target_p->form = ORIGIN_FORM;
                uri_parser_p->state = VALID_URI_TARGET;
                uri_parser_p->start_p = *cursor;
                return;
            }
        }
        (*cursor)++;
        }
    }
