#include "queue.h"
#include <string.h>  // for memcpy

// Initialize the generic queue
void queue_init(generic_queue_t *queue, void *buffer_storage, size_t element_size, size_t max_size) {
    queue->buffer_storage = buffer_storage;
    queue->element_size = element_size;
    queue->max_size = max_size;
    // Initialize the internal circular buffer
    circular_buffer_init(&queue->circular_buffer, buffer_storage, element_size, max_size);
}

// Push data into the queue
bool queue_push(generic_queue_t *queue, const void *data) {
    return circular_buffer_push(&queue->circular_buffer, data);
}

// Pop data from the queue
bool queue_pop(generic_queue_t *queue, void *data) {
    return circular_buffer_pop(&queue->circular_buffer, data);
}

// Check if the queue is full
bool queue_is_full(generic_queue_t *queue) {
    return circular_buffer_is_full(&queue->circular_buffer);
}

// Check if the queue is empty
bool queue_is_empty(generic_queue_t *queue) {
    return circular_buffer_is_empty(&queue->circular_buffer);
}
