// July 30, 2019

#include "shared.h"

#define HELP_MESSAGE \
    "Syntax: server FILE\n"

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
    if (argc < 2) {
        fprintf(stderr, "%s\n", HELP_MESSAGE);
        return EXIT_FAILURE;
    }

    FILE * file = open_file(argv[1]);
    int sock = bind_socket(1516);

    for (;;) {
        int peer = accept(sock, NULL, NULL);

        if (peer == -1) {
            die_errno("accept");
        }

        send_file(file, peer);
        close(peer);
    }

    close(sock);
    fclose(file);

    return EXIT_SUCCESS;
}
