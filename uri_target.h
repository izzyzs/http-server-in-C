#ifndef HTTP_URI_TARGET
#define HTTP_URI_TARGET

#include "key_value_list.h"

enum uri_target_form {
    URI_FORM_NOT_SET,
    ORIGIN_FORM,
    ABSOLUTE_FORM,
    AUTHORITY_FORM,
    ASTERISK_FORM
};

enum http_uri_scheme { SCHEME_NONE=-1, HTTP, HTTPS };

struct uri_host {
    char *text;
    enum { NO_URI_HOST_TYPE_DEFINED, IPv4, IPv6, IPvFuture, REG_NAME } host_type;
};

struct uri_origin_form {
    char *absolute_path;
    char *query;
};

struct uri_absolute_form {
    enum http_uri_scheme scheme;
    struct {
        struct {
            struct uri_host host;
            unsigned short port;
        } authority;
        char *path;
    } hier_part;
    char *query;
};

struct uri_authority_form {
    struct uri_host host;
    unsigned short port;
};

union uri_target_values {
        struct uri_origin_form origin_form;
        struct uri_absolute_form absolute_form;
        struct uri_authority_form authority_form;
    };

// the text found in request line
struct request_line_uri_target {
    enum uri_target_form form;
    union uri_target_values uri_target;
    
};

// the actual request target that can be incomplete 
// even if the uri-target found in the request line doesn't
// contain all the necessary information
struct http_resource_target {
    enum http_uri_scheme scheme;

};

enum http_uri_target_state {
    UT_START,
    UT_PARSE_FOR_ASTERISK_LOOK_FOR_SPACE,

    UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL,
    UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1,
    UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2,
    UT_PARSE_FOR_ORIGIN_QUERY,


    // I was thinking of handling all the possible cases required to afirm absolute form, then work with authority from there
    // UT_PARSE_FOR_H_IN_SCHEME_AF,
    UT_PARSE_FOR_FIRST_T_IN_SCHEME_ABSF,
    UT_PARSE_FOR_SECOND_T_IN_SCHEME_ABSF,
    UT_PARSE_FOR_P_IN_SCHEME_ABSF,
    UT_PARSE_FOR_S_OR_COLON_IN_SCHEME_ABSF,
    UT_PARSE_FOR_COLON_AFTER_S_IN_SCHEME_ABSF,
    UT_PARSE_FORWARD_SLASH_1_IN_HIER_PART_ABSF,
    UT_PARSE_FORWARD_SLASH_2_IN_HIER_PART_ABSF,
    UT_PARSE_HIER_AUTHORITY_HOST_ABSF,
    UT_PARSE_HIER_AUTHORITY_PORT_ABSF,
    UT_PARSE_HIER_PATH_ABSF,
    UT_PARSE_QUERY_ABSF,
    UT_PARSE_SPACE_ABSF,


    UT_PARSE_AUTHORITY_HOST_AUTHF,
    UT_PARSE_AUTHORITY_PORT_AUTHF,

    
    VALID_URI_TARGET,
    INVALID_URI_TARGET
};

enum abs_form_scheme_status {
    ABSF_SCHEME_INCOMPLETE,
    ABSF_SCHEME_NOT_FOUND,
    ABSF_SCHEME_FOUND,
};

struct http_uri_target_parser {
    enum http_uri_target_state state;
    char *start_p;
    int ip_delimiter_cnt;
    enum http_uri_scheme scheme_guess;
};

struct http_uri_target_parser init_http_uri_target_parser() {
    struct http_uri_target_parser parser = {
        .state = UT_START,
        .start_p = NULL,
        .scheme_guess = SCHEME_NONE
    };
    return parser;
}

int is_alpha(char c);
int is_digit(char c);
int is_unreserved_char(char c);
int is_hex_digit(char c);
int is_subdelimiter(char c);
int is_pchar(char c);

int parsing_for_asterisk_form(struct http_uri_target_parser *uri_parser_p, char **cursor);
int parsing_for_origin_form(struct http_uri_target_parser *uri_parser_p, char **cursor);
void init_target_struct_for_origin_target(struct request_line_uri_target *uri_target_p);
void parse_for_origin(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end);

#endif