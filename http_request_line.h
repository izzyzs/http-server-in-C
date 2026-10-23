#ifndef HTTP_REQUEST_LINE
#define HTTP_REQUEST_LINE

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

enum http_method { ERROR=-1, NONE, GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

struct http_request_line {
    enum http_method method;
    struct http_uri uri;
    struct http_version version;
};

enum http_request_line_status { ERROR=-1, HTTP_REQUEST_LINE_START, HTTP_REQUEST_LINE_METHOD_DONE,HTTP_REQUEST_LINE_URI_PATH_DONE, HTTP_REQUEST_LINE_URI_QUERY_DONE,HTTP_REQUEST_LINE_VERSION_DONE };
enum http_request_line_error { REQUEST_LINE_NO_ERR, REQUEST_LINE_METHOD_ERR, REQUEST_LINE_URI_ERR, REQUEST_LINE_VERSION_ERR };

struct http_request_line_state {  enum http_request_line_status status; enum http_request_line_error error; };

enum http_method_state {
    HTTP_METHOD_PARSE_ERR=-1, HTTP_METHOD_START, HTTP_METHOD_PARSED_AND_NOT_READ, HTTP_METHOD_PARSED_AND_READ
};
// enum http_uri_state {
//     HTTP_METHOD_PARSE_ERR=-1, HTTP_METHOD_START, HTTP_METHOD_PARSED_AND_NOT_READ, HTTP_METHOD_PARSED_AND_READ
// };

// struct http_request_line_state parse_request_line(struct http_request_line *request_line, char *cursor_p, char* end_p, int length);
struct http_request_line_state parse_request_line(struct http_request_line *request_line, struct http_request_line_state req_line_state, char **cursor_p, char *end_p);
char *req_line_err_to_str(enum http_request_line_status status);
enum http_method_state parse_request_method(char *start_pointer, char* end_pointer, enum http_method *method, enum http_method_state method_state);
int parse_uri(struct http_uri* uri, char *start_pointer);
struct key_value_list *parse_uri_query(char *start_pointer);
int parse_http_version(struct http_version *version, char *start_pointer);
char *http_method_to_str(enum http_method m);
void print_uri(struct http_uri u);
void print_http_version(struct http_version v);
void print_request_line(struct http_request_line request);

#endif