#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "key_value_list.h"
#include "http_request_line.h"

enum http_parser_state { START, REQUEST_LINE, FIELD_HEADERS, MESSAGE_BODY, COMPLETE, FAIL };
enum http_parser_event { COMPLETE_REQUEST_LINE, COMPLETE_FIELD_HEADERS, COMPLETE_MESSAGE_BODY, ERROR };
struct http_parser_state_machine { enum http_parser_state state; enum http_parser_event event; };
// static struct http_parser_state_machine transtion(struct http_parser_state_machine);
struct key_value_list *parse_field_lines(char *start_pointer);

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n);
// ssize_t robust_write(int file_descriptor, void *user_buffer, size_t n);

int main(void) {
    int fd = open("./request.txt", O_RDONLY, 0);
    char buffer[1024];
    int read = robust_read(fd, buffer, 1024);
    printf("read: %d\n", read);
    printf("%s\n", strerror(errno));
    return 0;
}

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n) {
    struct http_request_line request_line;
    enum http_request_line_status request_line_status;
    struct key_value_list *field_lines = create_key_value_list();
    

    ssize_t number_left = n;
    ssize_t number_read;
    char *buffer_pointer = user_buffer;
    char *start_of_line = user_buffer; // start of the last read line
    char *start_of_unread = user_buffer; // start of the unread portion of the application buffer
    
    while (number_left > 0) {
        if ((number_read = read(file_descriptor, buffer_pointer, number_left)) < 0) { // meaning if an error has occured while reading i.e., read() returns -1
            if (errno == EINTR) // check to see if the read system call has been blocked by an interrupt
            number_read = 0;
            else
            return -1; // errno has been set by read and there's some other reason for the error
        }
        else if (number_read == 0) // ran into an EOF
        break; // don't call it again;
        
        long i;
        number_left -= number_read; // subtract the number of bytes successfully read from the number of bytes left to be read
        buffer_pointer += number_read; // increment the buffer pointer the number_read amount of bytes.
        
        
        // process the newly read bytes
        parse(start_of_unread, &request_line, &request_line_status, &field_lines, user_buffer, buffer_pointer);
        char *charp_1 = start_of_unread, *charp_2 = start_of_unread+1;
        char char_1, char_2;
        for (i = (start_of_unread-(char*)user_buffer); i < (buffer_pointer-start_of_unread); i++) {
            char_1 = *charp_1;
            char_2 = *charp_2;
            // printf("processing bytes\n");
            // printf("i = %ld\n", i);
            // printf("\tchar_1 = %c\n\tchar_2 = %c\n", char_1, char_2);
            if (char_1 == '\r' && char_2 == '\n') {
                /*
                if parser.state = 
                */
               start_of_unread = charp_2 + 1;
               // printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)start_of_line);
               request_line_status = parse_request_line(&request_line, (char *)start_of_line, (int)(start_of_unread - start_of_line));
            } else {
                charp_1 += 1;
                charp_2 += 1;
            }
            
        }
        start_of_unread = buffer_pointer;
    }
    if (request_line_status != REQUEST_LINE_SUCCESS) {
        printf("%s\n", req_line_err_to_str(request_line_status));
    } else {
        printf("Successful parse:\n\n");
        print_request_line(request_line);
    }
    return (n - number_left); // returns the number of bytes filled bytes in the buffer in the case that there is nothing left to be read.
}

// static struct http_parser_state_machine transtion(struct http_parser_state_machine) {
//     switch (s) {
        
//     }
// }

void parse(const char* start_of_unread, struct http_request_line *request_line_p, enum http_request_line_status *request_line_status_p, struct key_value_list **field_lines_p, void *user_buffer, char *buffer_pointer) {
    char *char1_p = start_of_unread, *char2_p = start_of_unread + 1;
    char char_1, char_2;
    int i;
        for (i = (start_of_unread-(char*)user_buffer); i < (buffer_pointer-start_of_unread); i++) {
            char_1 = *char1_p;
            char_2 = *char2_p;
            // printf("processing bytes\n");
            // printf("i = %ld\n", i);
            // printf("\tchar_1 = %c\n\tchar_2 = %c\n", char_1, char_2);
            if (char_1 == '\r' && char_2 == '\n') {
                /*
                if parser.state = 
                */
               start_of_unread = char2_p + 1;
               // printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)start_of_line);
               *request_line_status_p = parse_request_line(request_line_p, (char *)start_of_line, (int)(start_of_unread - start_of_line));
            } else {
                char1_p += 1;
                char2_p += 1;
            }
            
        }
}