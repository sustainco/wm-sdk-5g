#ifndef _TOKEN_RECV_QUEUE_H
#define _TOKEN_RECV_QUEUE_H

#include <queue.h>

#define TOKEN_RCV_QUEUE_SIZE 5

typedef struct {
    uint64_t serial;
    char token[20];
}token_t;

extern generic_queue_t token_rcv_queue;

#endif //_TOKEN_RECV_QUEUE_H