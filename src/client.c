// July 30, 2019

#include "shared.h"

static char * hostname;
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

    hostname = argv[optind];
}

void recv_file(int sock) {
    char buffer[4096];
    long total_b = 0;
    message_t type;
    long data_len;
    struct timespec tp0, tp1;

    if (verbose) {
        clock_gettime(CLOCK_MONOTONIC, &tp0);
    }

    while ((data_len = recv_msg(sock, &type, buffer, sizeof(buffer))) > 0) {
        if (type != MSG_CHUNK) {
            die("Invalid message type: expecting chunk.");
        }

        size_t write_b = fwrite(buffer, 1, data_len, stdout);

        if (write_b != (size_t)data_len) {
            die_errno("fwrite");
        }

        total_b += data_len;

        if (delay) {
            sleep_ms(delay);
        }
    }

    if (data_len == 0) {
        if (type != MSG_END) {
            die("Invalid message type: expecting end.");
        }

        if (verbose) {
            clock_gettime(CLOCK_MONOTONIC, &tp1);
            double lapse = time_diff(tp0, tp1);
            fprintf(stderr, "Length: %zd bytes.\n", total_b);
            fprintf(stderr, "Time: %.03f seconds.\n", lapse);
            fprintf(stderr, "Throughput: %.03f Mbps.\n", total_b / lapse / 131072);
        }
    }
}

int main(int argc, char ** argv) {
    parse_options(argc, argv);

    int sock = connect_socket(hostname, 1516);
    recv_file(sock);

    close(sock);
    return EXIT_SUCCESS;
}
