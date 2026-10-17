#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>


typedef enum {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
} Method;

typedef struct {
    int major;
    int minor;
} Version;

typedef struct {
    char* absolute_path;
} URI;


typedef struct {
    Method method;
    URI uri;
    Version version;
} RequestLine;

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n);
ssize_t robust_write(int file_descriptor, void *user_buffer, size_t n);
Method parse_request_method(char* start_pointer);
URI parse_uri(char* start_pointer);
RequestLine parse_request_line(char* startp, int length);




int main(void) {
    int fd = open("./request.txt", O_RDONLY, 0);
    char buffer[1024];
    int read = robust_read(fd, buffer, 1024);
    printf("read: %d\n", read);
    printf("%s\n", strerror(errno));
    return 0;
}

ssize_t robust_read(int file_descriptor, void *user_buffer, size_t n) {

    ssize_t number_left = n;
    ssize_t number_read;
    char *buffer_pointer = user_buffer;
    char *start_of_unread = user_buffer; // start of the unread portion of the application buffer
    char *start_of_line = user_buffer; // start of the last read line

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

        printf("number_read: %zd\n", number_read);
        printf("buffer_pointer: %s\n\n", buffer_pointer);


        // process the newly read bytes
        char *char_1_pointer = start_of_unread, *char_2_pointer = start_of_unread+1, char_1, char_2;
        for (i = (start_of_unread-(char*)user_buffer); i < (buffer_pointer-start_of_unread); i++) {
            char_1 = *char_1_pointer;
            char_2 = *char_2_pointer;
            printf("processing bytes\n");
            printf("i = %ld\n", i);
            printf("\tchar_1 = %c\n\tchar_2 = %c\n", char_1, char_2);
             if (char_1 == '\r' && char_2 == '\n') {
                start_of_unread = char_2_pointer + 1;
                printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)start_of_line);
                parse_request_line((char *)start_of_line, (int)(start_of_unread - start_of_line));
                // printf("%.*s\n", (int)(start_of_unread - start_of_line), (char *)user_buffer + (start_of_unread - (char*)user_buffer));
            } else {
                char_1_pointer += 1;
                char_2_pointer += 1;
            }

        }
        start_of_unread = buffer_pointer;
    }
    return (n - number_left); // returns the number of bytes filled bytes in the buffer in the case that there is nothing left to be read.
}

Method parse_request_method(char *start_pointer) {
    printf("parse_request_method\n");
    printf("%c", *start_pointer);
    if (*start_pointer == 'G') {
        printf("GET\n");
        return GET;
    }
    if(*start_pointer == 'H') {
        printf("HEAD\n");
        return HEAD;
    }
    if (*start_pointer == 'P') {
        printf("%c\n", *(start_pointer+1));
        if (*(start_pointer + 1) == 'O') {
            printf("POST\n");
            return POST;
        } else {
            printf("PUT\n");
            return PUT;
        }
    }
    if(*start_pointer == 'D') {
        printf("DELETE\n");
        return DELETE;
    }
    if(*start_pointer == 'C') {
        printf("CONNECT\n");
        return CONNECT;
    }
    if(*start_pointer == 'O') {
        printf("OPTIONS\n");
        return OPTIONS;
    }
    if(*start_pointer == 'T') {
        printf("TRACE\n");
        return TRACE;
    }
    return GET;
}

URI parse_uri(char *start_pointer) {
    printf("parse_uri:\t");
    int length;
    char *cursor = start_pointer;
    while (*cursor != ' ') {
        cursor++;
    }
    length = cursor - start_pointer;
    printf("%.*s\n", length, start_pointer);
    printf("length: %d\n", length);
    char *uri_copy = (char*)malloc((length+1) * sizeof(char));
    memcpy(uri_copy, start_pointer, length);
    uri_copy[length] = '\0';
    URI uri = { uri_copy };
    return uri;
}

RequestLine parse_request_line(char* startp, int length) {
    Method method = parse_request_method(startp);
    while (*startp != ' ') {
        startp++;
    }
    startp++;
    URI uri = parse_uri(startp);
    while (*startp != ' ') {
        startp++;
    }
    startp++;

    RequestLine r = {method, uri, {1, 1}};
    return r;
}
