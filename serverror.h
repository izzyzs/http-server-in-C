#ifndef SERVERROR
#define SERVERROR

#include <stdio.h>
#include <errno.h>
#include <netdb.h>

const char *strerror_immutable(int err);
int print_server_error(char *msg);
int print_server_error_from_function(char *msg, int error_num, const char *(*print_func)(int));
int print_server_error_from_errno(char *msg);
int print_server_error_from_getaddrinfo(int error_num);

#endif