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
void * mem_alloc(size_t size);

void sleep_ms(int ms);

int bind_socket(unsigned short port);
int connect_socket(const char * hostname, unsigned short port);
void set_timeout(int sock, int seconds);
void set_nonblock(int sock);

typedef struct {
    uint32_t type;
    uint32_t size;
} header_t;

typedef enum { MSG_CHUNK, MSG_END } message_t;

long msg_pack(message_t type, const char * arg, long arg_len, char * payload, long payload_len);
long msg_unpack(const char * payload, long payload_len, message_t * type, char * arg, long * arg_len);
int send_msg(int sock, message_t type, const char * arg, long arg_len);
long recv_msg(int sock, message_t * type, char * arg, long arg_len);

#endif