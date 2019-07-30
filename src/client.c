// July 30, 2019

#include "shared.h"

static char * file;
static int delay;
static unsigned short port = 1516;
static bool verbose;

#define HELP_MESSAGE \
    "Syntax: client [ OPTIONS ] SERVER\n" \
    "Options:\n" \
    "   -d  <ms>    Delay milliseconds per each chunk.\n" \
    "   -h          Print this help.\n" \
    "   -p  <port>  Port number.\n" \
    "   -v          Be verbose."

void parse_options(int argc, char ** argv) {
    int opt;
    long value;
    char * end;

    while ((opt = getopt(argc, argv, "d:hp:v")) != -1) {
        switch (opt) {
        case 'd':
            value = strtol(optarg, &end, 10);

            if (value < 0 || value > INT_MAX || *end) {
                fprintf(stderr, "WARN: Ignoring invalid delay option value.\n");
            } else {
                delay = value;
            }
            
            break;

        case 'h':
            printf("%s\n", HELP_MESSAGE);
            exit(EXIT_SUCCESS);

        case 'p':
            value = strtol(optarg, &end, 10);

            if (value < 0 || value > USHRT_MAX || *end) {
                fprintf(stderr, "WARN: Ignoring invalid port option value.\n");
            } else {
                port = value;
            }
            
            break;

        case 'v':
            verbose = true;

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

void recv_file(int sock) {
    char buffer[4096];
    ssize_t recv_b;
    ssize_t total_b;
    struct timespec tp0, tp1;

    if (verbose) {
        clock_gettime(CLOCK_MONOTONIC, &tp0);
    }

    for (total_b = 0; (recv_b = recv(sock, buffer, sizeof(buffer), 0)) > 0; total_b += recv_b) {
        size_t write_b = fwrite(buffer, 1, recv_b, stdout);
        if (write_b != (size_t)recv_b) {
            die_errno("fwrite");
        }
    }

    if (recv_b == -1) {
        die_errno("recv");
    }

    if (verbose) {
        clock_gettime(CLOCK_MONOTONIC, &tp1);
        double lapse = time_diff(tp0, tp1);
        fprintf(stderr, "Length: %zd bytes.\n", total_b);
        fprintf(stderr, "Time: %.03f seconds.\n", lapse);
        fprintf(stderr, "Throughput: %.03f Mbps.\n", total_b / lapse / 131072);
    }
}

int main(int argc, char ** argv) {
    parse_options(argc, argv);

    int sock = connect_socket(file, 1516);
    recv_file(sock);

    close(sock);
    return EXIT_SUCCESS;
}
