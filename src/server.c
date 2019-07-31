// July 30, 2019

#include "shared.h"

#define HELP_MESSAGE \
    "Syntax: server [ OPTIONS ] FILE\n" \
    "Options:\n" \
    "   -h          Print this help.\n" \
    "   -t  <sec>   Delivery timeout.\n"

static char * file;
static int timeout;

void parse_options(int argc, char ** argv) {
    int opt;
    long value;
    char * end;

    while ((opt = getopt(argc, argv, "ht:")) != -1) {
        switch (opt) {
        case 'h':
            printf("%s\n", HELP_MESSAGE);
            exit(EXIT_SUCCESS);

        case 't':
            value = strtol(optarg, &end, 10);

            if (value < 0 || value > INT_MAX || *end) {
                fprintf(stderr, "WARN: Ignoring invalid timeout option value.\n");
            } else {
                timeout = value;
            }

            break;

        case '?':
            break;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "%s\n", HELP_MESSAGE);
        exit(EXIT_FAILURE);
    }

    file = argv[optind];
}

FILE * open_file(const char * path) {
    FILE * file = fopen(path, "r");

    if (file == NULL) {
        die_errno("fopen");
    }

    return file;
}

void send_file(FILE * file, int sock) {
    if (fseek(file, 0, SEEK_SET) == -1) {
        die_errno("fseek");
    }

    char buffer[4096];
    size_t read_b;

    printf("Sending file to client...\n");

    while ((read_b = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        struct timespec tp0, tp1;
        clock_gettime(CLOCK_MONOTONIC, &tp0);

        if (send_msg(sock, MSG_CHUNK, buffer, read_b) == -1) {
            return;
        }

        clock_gettime(CLOCK_MONOTONIC, &tp1);
        double lapse = time_diff(tp0, tp1);

        if (lapse > 1) {
            fprintf(stderr, "WARN: send() lasted %.03f seconds.\n", lapse);
        }
    }

    send_msg(sock, MSG_END, NULL, 0);

    printf("Delivery completed.\n");
}

int main(int argc, char ** argv) {
    parse_options(argc, argv);

    FILE * fp = open_file(file);
    int sock = bind_socket(1516);

    for (;;) {
        int peer = accept(sock, NULL, NULL);

        if (peer == -1) {
            die_errno("accept");
        }

        if (timeout) {
            set_timeout(peer, timeout);
        }

        send_file(fp, peer);
        close(peer);
    }

    close(sock);
    fclose(fp);

    return EXIT_SUCCESS;
}
