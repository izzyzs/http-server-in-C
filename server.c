#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "read_and_parse.h"
#include "serverror.h"

#define BACKLOG 1024
#define MAXLINE 500

void safe_close(int fd);


int create_listen_fd(struct addrinfo *list);

int main() {
    int status, listen_fd;
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char buff[MAXLINE];

    if ((status = getaddrinfo(NULL, "80", &hints, &server_info) != 0)) {
        print_server_error_from_getaddrinfo(status);
        exit(1);
    }

    if ((listen_fd = create_listen_fd(server_info)) == -1) {
        print_server_error("failed to create listening file descriptor (create_listen_fd)");
    }
    freeaddrinfo(server_info);

    if (listen(listen_fd, BACKLOG) == -1) {
        print_server_error_from_errno("listen");
    }

    for(;;) {
        struct sockaddr clientaddr;
        socklen_t clientaddrlen;
        int connect_fd;
        safe_accept(listen_fd, &clientaddr, &clientaddrlen, &connect_fd);
        if (!connect_fd) {
            print_server_error("Couldn't connect to client, try again");
            continue;
        }
        
        
    }

}

int create_listen_fd(struct addrinfo *list) {
    int listen_fd, enable_option = 1;
    struct addrinfo *curr;
    for (curr = list; curr; curr = curr->ai_next) {
        if ((listen_fd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) == -1) {
            print_server_error_from_errno("socket() listen_fd");
            continue;
        }

        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable_option, sizeof(int)) == -1) {
            print_server_error_from_errno("setsockopt (SO_REUSEADDR -> TRUE)");
            safe_close(listen_fd);
            continue;
        }
        
        if (bind(listen_fd, curr->ai_addr, curr->ai_addrlen) == -1) {
            print_server_error_from_errno("bind");
            safe_close(listen_fd);
            continue;
        }
        return listen_fd;
    }
    return -1;
}


void safe_close(int fd) {
    if (close(fd) == -1) {
        print_server_error_from_errno("close");
    }
}

void safe_accept(int fd, struct sockaddr *__restrict addr, socklen_t *__restrict addrlen, int *connect_fd_p) {
    int connect_fd;
    if ((connect_fd = accept(fd, addr, addrlen)) == -1) {
        print_server_error_from_errno("accept");
        connect_fd_p = NULL;
        return;
    }
    *connect_fd_p = connect_fd;
    return;
}