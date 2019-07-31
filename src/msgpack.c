// July 30, 2019

#include "shared.h"

long msg_pack(message_t type, const char * arg, long arg_len, char * payload, long payload_len) {
    header_t header = { type, arg_len };

    if (payload_len < arg_len + (long)sizeof(header_t)) {
        return -1;
    }

    *(header_t *)payload = header;

    if (arg_len > 0) {
        memcpy(payload + sizeof(header_t), arg, arg_len);
    }

    return sizeof(header_t) + arg_len;
}

long msg_unpack(const char * payload, long payload_len, message_t * type, char * arg, long * arg_len) {
    if (payload_len < (long)sizeof(header_t)) {
        return -1;
    }

    header_t header = *(header_t *)payload;
    long offset = sizeof(header_t) + header.size;

    if (offset > payload_len || (long)header.size > *arg_len) {
        return -1;
    }

    *type = header.type;
    *arg_len = header.size;
    memcpy(arg, payload + sizeof(header_t), header.size);

    return offset;
}