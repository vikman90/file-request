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

int send_msg(int sock, message_t type, const char * arg, long arg_len) {
    long payload_len = sizeof(header_t) + arg_len;
    char * payload = mem_alloc(payload_len);

    payload_len = msg_pack(type, arg, arg_len, payload, payload_len);

    if (payload_len == -1) {
        die("Cannot build a payload.");
    }

    ssize_t send_b;
    ssize_t send_acc;

    for (send_acc = 0; send_acc < payload_len; send_acc += send_b) {
        send_b = send(sock, payload + send_acc, payload_len - send_acc, 0);

        switch (send_b) {
        case -1:
            switch (errno) {
            case ECONNRESET:
                printf("Client closed the connection.\n");
                goto cleanup;

            default:
                die_errno("send");
            }

        case 0:
            fprintf(stderr, "WARN: No data sent. Retrying.\n");
            break;

        default:
            if (send_b < payload_len) {
                fprintf(stderr, "WARN: Data partially delivered: %zd bytes\n", send_b);
            }
        }
    }

cleanup:
    free(payload);
    return send_acc == payload_len ? 0 : -1;
}

long recv_msg(int sock, message_t * type, char * arg, long arg_len) {
    header_t header;

    ssize_t recv_b = recv(sock, &header, sizeof(header), MSG_WAITALL);

    switch (recv_b) {
    case -1:
        die_errno("recv");

    case 0:
        fprintf(stderr, "WARN: Connection closed.\n");
        return -1;

    default:
        if (recv_b != sizeof(header)) {
            die("Incomplete message received.");
        }
    }

    if (!(header.type == MSG_CHUNK || header.type == MSG_END)) {
        die("Invalid header: unknown type.");
    }

    if (header.size > arg_len) {
        die("Invalid header: message seems too large.");
    }

    if (header.size > 0) {
        recv_b = recv(sock, arg, header.size, MSG_WAITALL);

        switch (recv_b) {
        case -1:
            die_errno("recv");

        case 0:
            fprintf(stderr, "WARN: Connection closed.\n");
            return -1;

        default:
            if (recv_b != header.size) {
                die("Incomplete message received.");
            }
        }
    }

    *type = header.type;
    return header.size;
}

