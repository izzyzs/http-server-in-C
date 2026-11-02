#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include "serverror.h"

const char *strerror_immutable(int err) {
    return strerror(err);
}

int print_server_error(char *msg) {
    return fprintf(stderr, "SERVER ERROR: %s\n", msg);
}

int print_server_error_from_function(char *msg, int error_num, const char *(*print_func)(int)) {
    return fprintf(stderr, "SERVER ERROR (%s): %s\n", msg, print_func(error_num));
}

int print_server_error_from_errno(char *msg) {
    return print_server_error_from_function(msg, errno, strerror_immutable);
}

int print_server_error_from_getaddrinfo(int error_num) {
    return print_server_error_from_function("getaddrinfo", error_num, gai_strerror);
}

