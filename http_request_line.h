#ifndef HTTP_REQUEST_LINE
#define HTTP_REQUEST_LINE

#include "uri_target.h"
#include "key_value_list.h"

struct http_version {
    int major;
    int minor;
};

struct http_uri {
    char *absolute_path;
    struct key_value_list *query_values;
};

enum http_method { HTTP_METHOD_ERROR=-1, NONE, GET, HEAD, POST, PUT, PATCH, DELETE, CONNECT, OPTIONS, TRACE };

struct http_request_line {
    enum http_method method;
    struct http_uri uri;
    struct http_version version;
};


enum http_request_line_state { 
    HTTP_REQUEST_LINE_START, 
    HTTP_REQUEST_LINE_HANDLING_METHOD,
    HTTP_REQUEST_LINE_HANDLING_URI_TARGET,
    HTTP_REQUEST_LINE_HANLDING_VERSION
};


enum http_method_state {
        METHOD_STATE_START,
        
        //         case GET:
        READ_G_EXPECTING_E_IN_GET, 
        READ_E_EXPECTING_T_IN_GET, 
        READ_T_EXPECTING_SPACE_AFTER_GET,
        
        //         case HEAD:
        READ_H_EXPECTING_E_IN_HEAD,
        READ_E_EXPECTING_A_IN_HEAD,
        READ_A_EXPECTING_D_IN_HEAD,
        READ_D_EXPECTING_SPACE_AFTER_HEAD,
        
        
        // case POST || PUT:
        READ_P_EXPECTING_EITHER_O_OR_U_OR_A,
        //         case POST:
        READ_O_EXPECTING_S_IN_POST,
        READ_S_EXPECTING_T_IN_POST,
        READ_T_EXPECTING_SPACE_AFTER_POST,
        //         case PUT:
        READ_U_EXPECTING_T_IN_PUT,
        READ_T_EXPECTING_SPACE_AFTER_PUT,
        //         case PATCH:
        READ_A_EXPECTING_T_IN_PATCH,
        READ_T_EXPECTING_C_IN_PATCH,
        READ_C_EXPECTING_H_IN_PATCH,
        READ_H_EXPECTING_SPACE_AFTER_PATCH,

        
        //         case DELETE:
        READ_D_EXPECTING_FIRST_E_IN_DELETE,
        READ_FIRST_E_EXPECTING_L_IN_DELETE,
        READ_L_EXPECTING_SECOND_E_IN_DELETE,
        READ_SECOND_E_EXPECTING_T_IN_DELETE,
        READ_T_EXPECTING_THIRD_E_IN_DELETE,
        READ_THIRD_E_EXPECTING_SPACE_AFTER_DELETE,
        
        //         case CONNECT:
        READ_FIRST_C_EXPECTING_O_IN_CONNECT,
        READ_O_EXPECTING_FIRST_N_IN_CONNECT,
        READ_FIRST_N_EXPECTING_SECOND_N_IN_CONNECT,
        READ_SECOND_N_EXPECTING_E_IN_CONNECT,
        READ_E_EXPECTING_SECOND_C_IN_CONNECT,
        READ_SECOND_C_EXPECTING_T_IN_CONNECT,
        READ_T_EXPECTING_SPACE_AFTER_CONNECT,
        
        // case OPTIONS:
        READ_FIRST_O_EXPECTING_P_IN_OPTIONS,
        READ_P_EXPECTING_T_IN_OPTIONS,
        READ_T_EXPECTING_I_IN_OPTIONS,
        READ_I_EXPECTING_SECOND_O_IN_OPTIONS,
        READ_SECOND_O_EXPECTING_N_IN_OPTIONS,
        READ_N_EXPECTING_S_IN_OPTIONS,
        READ_S_EXPECTING_SPACE_AFTER_OPTIONS,

        // case TRACE:
        READ_T_EXPECTING_R_IN_TRACE,
        READ_R_EXPECTING_A_IN_TRACE,
        READ_A_EXPECTING_C_IN_TRACE,
        READ_C_EXPECTING_E_IN_TRACE,
        READ_E_EXPECTING_SPACE_AFTER_TRACE,

        VALID_METHOD,
        INVALID_METHOD
};

enum http_version_state {    
    HTTP_VERSION_START,
};

struct http_field_lines_parser {};

struct http_req_line_parser {
    enum http_request_line_state self_state;
    enum http_method_state method_state;
    struct http_uri_target_parser target_parser;
    enum http_version_state version_state;
    struct http_field_lines_parser field_lines_parser;
};

int http_req_line_parse_is_err(enum http_request_line_state);


// enum http_uri_target_state {
//     HTTP_METHOD_PARSE_ERR=-1, HTTP_METHOD_START, HTTP_METHOD_PARSED_AND_NOT_READ, HTTP_METHOD_PARSED_AND_READ
// };


enum http_request_line_state parse_request_line(struct http_request_line *request_line, enum http_request_line_state req_line_state, char **cursor_p, char *end_p);

void parse_request_method(struct http_req_line_parser *req_line_parser, char **cursor, char *end, enum http_method *method);
int method_parsing_not_done(enum http_method_state method_state);
int method_parsing_for_GET(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_GET(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_HEAD(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_HEAD(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_PUT_or_POST_or_PATCH(enum http_method_state *method_state_p, char **cursor);
int method_parsing_for_PUT(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_PUT(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_POST(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_POST(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_PATCH(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_PATCH(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_DELETE(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_DELETE(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_CONNECT(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_CONNECT(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_OPTIONS(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_OPTIONS(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);
int method_parsing_for_TRACE(enum http_method_state *method_state_p, char **cursor);
void parse_method_for_TRACE(enum http_method *http_method_p, enum http_method_state *method_state_p, char **cursor, char *end);

int parse_uri(struct http_uri* uri, char *start_pointer);
struct key_value_list *parse_uri_query(char *start_pointer);
int parse_http_version(struct http_version *version, char *start_pointer, struct http_req_line_parser *req_line_parser);
char *http_method_to_str(enum http_method m);
void print_uri(struct http_uri u);
void print_http_version(struct http_version v);
void print_request_line(struct http_request_line request);

#endif