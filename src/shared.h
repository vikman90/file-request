// July 30, 2019

#ifndef SHARED_H
#define SHARED_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define time_diff(tp0, tp1) (tp1.tv_sec - tp0.tv_sec + (tp1.tv_nsec - tp0.tv_nsec) / 1e9)

void die(const char * s) __attribute__ ((noreturn));
void die_errno(const char * s) __attribute__ ((noreturn));

void sleep_ms(int ms);

int bind_socket(unsigned short port);
int connect_socket(const char * hostname, unsigned short port);
void set_timeout(int sock, int seconds);
void set_nonblock(int sock);

typedef enum { CHUNK, END } message_t;

#endif