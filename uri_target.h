#ifndef HTTP_URI_TARGET
#define HTTP_URI_TARGET

#include "key_value_list.h"

enum uri_target_form {
    ORIGIN_FORM,
    ABSOLUTE_FORM,
    AUTHORITY_FORM,
    ASTERISK_FORM
};

enum http_uri_scheme { HTTP, HTTPS };

struct uri_origin_form {
    char *absolute_path;
    struct key_value_list *query;
};

struct uri_absolute_form {
    enum http_uri_scheme scheme;
    struct {
        char *authority;
        char *path;
    } hier_part;
    char *query;
};

struct uri_authority_form {
    struct {
        char *text;
        enum { IPv4, IPv6, IPvFuture, REG_NAME } host_type;
    } uri_host;
    unsigned short port;
};

// the text found in request line
struct request_line_uri_target {
    enum uri_target_form form;
    union
    {
        struct uri_origin_form origin_form;
        struct uri_absolute_form absolute_form;
        struct uri_authority_form authority_form;
    } uri_target;
    
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

    UT_PARSE_FOR_ORIGIN_ABS_PATH,
    UT_PARSE_FOR_ORIGIN_QUERY,
    

    VALID_URI_TARGET,
    INVALID_URI_TARGET
};

struct http_uri_target_parser {
    enum http_uri_target_state state;
    char *start_p;
};

#endif