#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdbool.h>
#include <stddef.h> // for size_t

#include <circular_buffer.h>

// Queue structure based on circular buffer
typedef struct {
    circular_buffer_t circular_buffer;  // Circular buffer for internal storage
    void *buffer_storage;               // Generic pointer to the actual queue data storage
    size_t element_size;                // Size of each element in the queue
    size_t max_size;                    // Maximum number of elements in the queue
} generic_queue_t;

// Initialize the generic queue
void queue_init(generic_queue_t *queue, void *buffer_storage, size_t element_size, size_t max_size);

// Push data into the queue
bool queue_push(generic_queue_t *queue, const void *data);

// Pop data from the queue
bool queue_pop(generic_queue_t *queue, void *data);

// Check if the queue is full
bool queue_is_full(generic_queue_t *queue);

// Check if the queue is empty
bool queue_is_empty(generic_queue_t *queue);

#endif // _QUEUE_H
