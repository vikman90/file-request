// July 30, 2019

#include "shared.h"

void die(const char * s) {
    fprintf(stderr, "ERROR: %s\n", s); 
    exit(EXIT_FAILURE);
}

void die_errno(const char * s) {
    perror(s);
    abort();
}

void sleep_ms(int ms) {
    struct timeval timeout = { ms / 1000, (ms % 1000) * 1000 };
    select(0, NULL, NULL, NULL, &timeout);
}

void * mem_alloc(size_t size) {
    void * memory = malloc(size);

    if (memory == NULL) {
        die_errno("Cannot allocate memory");
    }

    return memory;
}