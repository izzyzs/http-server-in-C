#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "uri_target.h"
#include "key_value_list.h"
#include "serverror.h"


void handle_test_list(const char **uris, size_t list_len);
char *parser_state_str(struct http_uri_target_parser parser);
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
enum abs_form_scheme_status try_for_scheme(struct http_uri_target_parser *uri_parser_p, char **cursor);

int main() {
    test_origin_parser();
    
    return 0;
}

enum abs_form_scheme_status try_for_scheme(struct http_uri_target_parser *uri_parser_p, char **cursor) {
    if (uri_parser_p->state == UT_START) uri_parser_p->start_p = *cursor;
    char lowered = tolower(**cursor);
    if (lowered == 'h' && uri_parser_p->state == UT_START) {
        uri_parser_p->state = UT_PARSE_FOR_FIRST_T_IN_SCHEME_ABSF;
        (*cursor)++;
        return ABSF_SCHEME_INCOMPLETE;
    } else if (lowered == 't' && uri_parser_p->state == UT_PARSE_FOR_FIRST_T_IN_SCHEME_ABSF) {
        uri_parser_p->state = UT_PARSE_FOR_SECOND_T_IN_SCHEME_ABSF;
        (*cursor)++;
        return ABSF_SCHEME_INCOMPLETE;
    } else if (lowered == 't' && uri_parser_p->state == UT_PARSE_FOR_SECOND_T_IN_SCHEME_ABSF) {
        uri_parser_p->state = UT_PARSE_FOR_P_IN_SCHEME_ABSF;
        (*cursor)++;
        return ABSF_SCHEME_INCOMPLETE;
    } else if (lowered == 'p' && uri_parser_p->state == UT_PARSE_FOR_P_IN_SCHEME_ABSF) {
        uri_parser_p->state = UT_PARSE_FOR_S_OR_COLON_IN_SCHEME_ABSF;
        (*cursor)++;
        return ABSF_SCHEME_INCOMPLETE;
    } else if (uri_parser_p->state == UT_PARSE_FOR_S_OR_COLON_IN_SCHEME_ABSF) {
        if (lowered == 's') {
            uri_parser_p->state = UT_PARSE_FOR_COLON_AFTER_S_IN_SCHEME_ABSF;
            (*cursor)++;
            return ABSF_SCHEME_INCOMPLETE;
        } else if (**cursor == ':') {
            uri_parser_p->state = UT_PARSE_FORWARD_SLASH_1_IN_HIER_PART_ABSF;
            uri_parser_p->scheme_guess = HTTP;
            (*cursor)++;
            return ABSF_SCHEME_FOUND;
        }
    } else if (uri_parser_p->state == UT_PARSE_FOR_COLON_AFTER_S_IN_SCHEME_ABSF && **cursor == ':') {
        uri_parser_p->state = UT_PARSE_FORWARD_SLASH_1_IN_HIER_PART_ABSF;
        uri_parser_p->scheme_guess = HTTPS;
        (*cursor)++;
        return ABSF_SCHEME_FOUND;
    }
    return ABSF_SCHEME_NOT_FOUND;
}

int parsing_for_absolute_or_authority(struct http_uri_target_parser *uri_parser_p, char **cursor, char *end) {
    // I was thinking of handling all the possible cases required to affirm absolute form, then work with authority from there
    // started writing the states for HTTP/HTTPS in uri_target.h
    // we can also parse and keep count, if u pass 4 letters that equal http or 5 letters that equal https and you don't find a semicolon followed with numbers it can't be authority afterward, then it can't be 
    // basically for authority parse for everything up until a colon, and if only numbers that don't fall in the allowable range for ports don't come after the colon then sayonara, it isn't authority
    // so maybe a parsing for authority or absolute function might be more proper, and we parse. based on what isn't allowed vs what is.
    enum abs_form_scheme_status scheme_status = ABSF_SCHEME_INCOMPLETE;
    
    do {
        scheme_status = try_for_scheme(uri_parser_p, cursor);
    } while (scheme_status == ABSF_SCHEME_INCOMPLETE);
    
    if (scheme_status == ABSF_SCHEME_FOUND) {
        if (uri_parser_p->state == UT_PARSE_FORWARD_SLASH_1_IN_HIER_PART_ABSF && **cursor == '/') {
           uri_parser_p->state = UT_PARSE_FORWARD_SLASH_2_IN_HIER_PART_ABSF;
           (*cursor)++;
           return 1;
        } else if (uri_parser_p->state == UT_PARSE_FORWARD_SLASH_2_IN_HIER_PART_ABSF && **cursor == '/') {
            uri_parser_p->state = UT_PARSE_AUTHORITY_HOST_ABSF;
            uri_parser_p->start_p = *cursor;
            (*cursor)++;
           return 1;
       }
    }
    return 0;
}

void parse_for_authority(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end) {

}

// the uri strings were produced by ChatGPT to help with testing
void test_origin_parser() {

const char *valid_uris[] = {
    // Authority-form
    // "example.com:443  ",
    // "localhost:8080  ",
    // "127.0.0.1:3000  ",
    // "api.example.com:8443  ",
    // "[::1]:8080  ",
    // "[2001:db8::1]:443  ",

    // Absolute-form
    // "http://example.com  ",
    // "http://example.com/  ",
    // "https://example.com/index.html  ",
    // "http://localhost:8080/api/users  ",
    // "http://example.com:3000/test  ",
    // "http://example.com/search?q=hello  ",
    // "https://example.com/a/b/c?x=1&y=2  ",
    // "http://127.0.0.1:8080/  ",
    // "http://[::1]:8080/test  ",
    // "HTTP://EXAMPLE.COM/path  ",

    // Asterisk-form
    // "*  ",

    // Origin-form
    "/",
    "/index.html",
    "/api/users",
    "/api/users/123",
    "/search?q=hello",
    "/?key=value",
    "/path/to/resource?x=1&y=2",
    "/file%20name.txt",
    "/a/b/../c",
    "//double/slash",
    "/path?",
    "/path?redirect=http://example.com"
};

// const size_t valid_uris_count =
//     sizeof(valid_uris) / sizeof(valid_uris[0]);

    const char *invalid_uris[] = {
    //     "hello HTTP/1.1. ",
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
    // size_t invalid_len = sizeof(invalid_uris)/sizeof(invalid_uris[0]);
    printf("::::::::::\n");
    printf("VALID URIS\n");
    printf("::::::::::\n");
    handle_test_list(valid_uris, valid_len);
    // printf("::::::::::::\n");
    // printf("INVALID URIS\n");
    // printf("::::::::::::\n");;
    // handle_test_list(invalid_uris, invalid_len);
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
        int parsing_for_absolute;

        uri_parser_p->state = UT_START;
        if (parsing_for_asterisk_form(uri_parser_p, cursor)) {
            printf("asterisk form\n");
            reql_uri_target_p->form = ASTERISK_FORM;
            uri_parser_p->state = VALID_URI_TARGET;
            // TODO: will have to check for method = OPTIONS and uri_target = ASTERISK match on return
            (*cursor)++;
            
        } else if (parsing_for_origin_form(uri_parser_p, cursor)) {
            printf("parsing for origin\n");
            parse_for_origin(reql_uri_target_p, uri_parser_p, cursor, end);
        } 
        // else {
        //     parsing_for_absolute = parsing_for_absolute_or_authority(uri_parser_p, cursor, end);
        //     if (parsing_for_absolute) {
        //         // parse_For_absolute()
        //         printf("parsing for absolute\n");
        //     } else {
        //         // parse_for_authority()
        //         printf("parsing for authority\n");
        //     }
        // }

        print_uri(reql_uri_target_p);
        printf("DEBUG: parser state -> %s\n\n", parser_state_str(*uri_parser_p));
    }
}

char *parser_state_str(struct http_uri_target_parser parser) {
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
        case UT_PARSE_FOR_FIRST_T_IN_SCHEME_ABSF:
            return "UT_PARSE_FOR_FIRST_T_IN_SCHEME_ABSF";
        case UT_PARSE_FOR_SECOND_T_IN_SCHEME_ABSF:
            return "UT_PARSE_FOR_SECOND_T_IN_SCHEME_ABSF";
        case UT_PARSE_FOR_P_IN_SCHEME_ABSF:
            return "UT_PARSE_FOR_P_IN_SCHEME_ABSF";
        case UT_PARSE_FOR_S_OR_COLON_IN_SCHEME_ABSF:
            return "UT_PARSE_FOR_S_OR_COLON_IN_SCHEME_ABSF";
        case UT_PARSE_FOR_COLON_AFTER_S_IN_SCHEME_ABSF:
            return "UT_PARSE_FOR_COLON_AFTER_S_IN_SCHEME_ABSF";
        case UT_PARSE_FORWARD_SLASH_1_IN_HIER_PART_ABSF:
            return "UT_PARSE_FORWARD_SLASH_1_IN_HIER_PART_ABSF";
        case UT_PARSE_FORWARD_SLASH_2_IN_HIER_PART_ABSF:
            return "UT_PARSE_FORWARD_SLASH_2_IN_HIER_PART_ABSF";
        case UT_PARSE_AUTHORITY_HOST_ABSF:
            return "UT_PARSE_AUTHORITY_HOST_ABSF";
        case UT_PARSE_AUTHORITY_PORT_ABSF:
            return "UT_PARSE_AUTHORITY_PORT_ABSF";
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
            printf("URI Not printable\n");
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
    
    int run = 0;
    while (*cursor >= end) {
        // printf("run #%d\tcursor: %c\n", ++run, **cursor);
        // printf("parser state: %s\n", parser_state_str(*uri_parser_p));
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
                        // printf("cursor == space and uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL");
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
