#ifndef _CIRCULAR_BUFFER_H
#define _CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stddef.h> // for size_t
#include <stdint.h>

// Circular buffer structure
typedef struct {
    void *buffer;     // Buffer memory to hold data
    size_t element_size;  // Size of each element in the buffer
    size_t max_elements;  // Maximum number of elements the buffer can hold
    size_t head;      // Index of the head (next item to pop)
    size_t tail;      // Index of the tail (next empty spot to push)
    bool is_full;     // Flag to indicate if the buffer is full
} circular_buffer_t;

// Initialize the circular buffer
void circular_buffer_init(circular_buffer_t *cb, void *buffer, size_t element_size, size_t max_elements);

// Push an element into the buffer
bool circular_buffer_push(circular_buffer_t *cb, const void *item);

// Pop an element from the buffer
bool circular_buffer_pop(circular_buffer_t *cb, void *item);

// Check if the buffer is full
bool circular_buffer_is_full(circular_buffer_t *cb);

// Check if the buffer is empty
bool circular_buffer_is_empty(circular_buffer_t *cb);

#endif // _CIRCULAR_BUFFER_H
