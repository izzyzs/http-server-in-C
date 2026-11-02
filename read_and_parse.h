#ifndef READ_N_PARSE
#define READ_N_PARSE


#include <unistd.h>
#include "key_value_list.h"
#include "http_request_line.h"
#include "http_field_lines.h"


enum http_request_parser_state {
    HTTP_REQUEST_PARSER_START
};


struct http_request_parser {
    enum http_request_parser_state self_state;
    struct http_req_line_parser req_line_parser;
};


enum socket_reader_state { READ_ERROR, BUFFER_COMPLETE_FILL, BUFFER_PARTIAL_FILL, CONN_CLOSED };
enum http_request_parser_state {
    HTTP_PARSER_START,
    HTTP_PARSER_REQUEST_LINE_METHOD_DONE,
    HTTP_PARSER_REQUEST_LINE_URI_PATH_DONE,
    HTTP_PARSER_REQUEST_LINE_URI_QUERY_DONE,
    HTTP_PARSER_REQUEST_LINE_VERSION_DONE,
    HTTP_PARSER_REQUEST_FIELD_HEADERS_DONE,
    HTTP_PARSER_REQUEST_MSG_BODY_DONE,
    HTTP_PARSER_COMPLETE,
    HTTP_PARSE_ERROR
};
enum http_request_parser_event { HTTP_PARSEER_NONE, COMPLETE_REQUEST_LINE, COMPLETE_FIELD_HEADERS, COMPLETE_MESSAGE_BODY, HTTP_REQUEST_PARSER_ERROR };
struct http_request_parser_state_machine { enum http_request_parser_state state; enum http_request_parser_event event; };
struct http_request {
    struct http_request_line request_line;
};

enum socket_reader_state check_read_state(ssize_t read_len, size_t buffer_size);
struct http_request_parser_state_machine initialize_http_parser_state_machine();

void parse(char *buffer, size_t end_of_read, struct http_request_parser_state_machine *parser_state_mach_p, struct http_request *request_p);
void read_and_parse(int file_desc, char buffer[], ssize_t buffer_size);


#endif