// July 30, 2019

#include "shared.h"

int bind_socket(unsigned short port) {
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = port };

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        die_errno("socket");
    }

    int flag = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
        die_errno("setsockopt");
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        die_errno("bind");
    }

    if (listen(sock, SOMAXCONN) == -1) {
        die_errno("listen");
    }

    return sock;
}

int connect_socket(const char * hostname, unsigned short port) {
    struct hostent * hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        fprintf(stderr, "Host name not found.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = port, .sin_addr = *(struct in_addr *)hostent->h_addr };

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        die_errno("socket");
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        switch (errno) {
        case ECONNREFUSED:
            die("Cannot connect to server.");
        default:
            die_errno("connect");
        }
    }

    return sock;
}

void set_timeout(int sock, int seconds) {
    struct timeval timeout = { .tv_sec = seconds };

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
        die_errno("setsockopt(SO_SNDTIMEO)");
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        die_errno("setsockopt(SO_RCVTIMEO)");
    }
}

void set_nonblock(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);

    if (flags == -1) {
        die_errno("fcntl(F_GETFL)");
    }

    if ((flags & O_NONBLOCK) == O_NONBLOCK) {
        printf("INFO: The socket was already in non-blocking mode.\n");
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        die_errno("fcntl(F_SETFL)");
    }
}

