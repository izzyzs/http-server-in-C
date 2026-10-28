#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "key_value_list.h"
#include "http_request_line.h"

#define BUFF_SIZE 1024

enum socket_reader_state { READ_ERROR, BUFFER_COMPLETE_FILL, BUFFER_PARTIAL_FILL, CONN_CLOSED };
enum http_request_parser_state {
    START,
    HTTP_REQUEST_LINE_METHOD_DONE,
    HTTP_REQUEST_LINE_URI_PATH_DONE,
    HTTP_REQUEST_LINE_URI_QUERY_DONE,
    HTTP_REQUEST_LINE_VERSION_DONE,
    HTTP_REQUEST_FIELD_HEADERS_DONE,
    HTTP_REQUEST_MSG_BODY_DONE,
    COMPLETE,
    HTTP_PARSE_ERROR
};
enum http_request_parser_event { NONE, COMPLETE_REQUEST_LINE, COMPLETE_FIELD_HEADERS, COMPLETE_MESSAGE_BODY, ERROR };
struct http_request_parser_state_machine { enum http_request_parser_state state; enum http_request_parser_event event; };
struct http_request {
    struct http_request_line request_line;
};

enum socket_reader_state check_read_state(ssize_t read_len, size_t buffer_size);
struct http_request_parser_state_machine initialize_http_parser_state_machine();

void parse(char *buffer, size_t end_of_read, struct http_request_parser_state_machine *parser_state_mach_p, struct http_request *request_p);

int main() {
    int fd = open("./request.txt", O_RDONLY, 0);
    char buffer[BUFF_SIZE];
    read_and_parse(fd, buffer, BUFF_SIZE);
}

void read_and_parse(int file_desc, char buffer[], ssize_t buffer_size) {
    struct http_request_parser_state_machine parser_state_machine = initialize_http_parser_state_machine();
    struct http_request request;

    for (;;) {
        ssize_t read_len = recv(file_desc, buffer, buffer_size);
        enum socket_reader_state reader_state = check_read_state(read_len, buffer_size);
        if (reader_state == READ_ERROR) {
            fprintf(stderr, "READ ERROR: error reading from socket. Shutting down...\n");
            break;
        } else if (reader_state == CONN_CLOSED) {
            fprintf(stdout, "Client closed connection. Shutting down...\n");
            break;
        }
        parse(buffer, read_len, &parser_state_machine, &request);
    }
}

void parse(char *buffer, size_t end_of_read, struct http_request_parser_state_machine *parser_state_mach_p, struct http_request *request_p) {
    char *cursor = buffer;
    char *end = buffer[end_of_read];
    struct http_request_line req_line;
    struct http_request_line_state req_line_state = { HTTP_REQUEST_LINE_START, REQUEST_LINE_NO_ERR };

    while (cursor != end) {
        switch (parser_state_mach_p->state) {
            case START:
            case HTTP_REQUEST_LINE_METHOD_DONE:
            case HTTP_REQUEST_LINE_URI_PATH_DONE:
            case HTTP_REQUEST_LINE_URI_QUERY_DONE:
            case HTTP_REQUEST_LINE_VERSION_DONE:
                req_line_state = parse_request_line(&req_line, req_line_state, &cursor, end);
                
        }
    }
}


enum socket_reader_state check_read_state(ssize_t read_len, size_t buffer_size) {
        if (read_len < 0) {
            return READ_ERROR;
        } else if ((size_t)read_len == buffer_size) {
            return BUFFER_COMPLETE_FILL;
        } else if (read_len == 0) {
            return CONN_CLOSED;
        }
        return BUFFER_PARTIAL_FILL;
    }
}

struct http_request_parser_state_machine initialize_http_parser_state_machine() {
    struct http_request_parser_state_machine machine = {START, NONE};
    return machine;
}
