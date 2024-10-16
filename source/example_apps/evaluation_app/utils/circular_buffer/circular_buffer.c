#include <circular_buffer.h>
#include <string.h>  // for memcpy

// Initialize the circular buffer
void circular_buffer_init(circular_buffer_t *cb, void *buffer, size_t element_size, size_t max_elements) {
    cb->buffer = buffer;
    cb->element_size = element_size;
    cb->max_elements = max_elements;
    cb->head = 0;
    cb->tail = 0;
    cb->is_full = false;
}

// Push an element into the buffer
bool circular_buffer_push(circular_buffer_t *cb, const void *item) {
    if (circular_buffer_is_full(cb)) {
        return false; // Buffer is full
    }

    // Copy item to the tail of the buffer
    memcpy((uint8_t *)cb->buffer + (cb->tail * cb->element_size), item, cb->element_size);

    // Move the tail to the next position
    cb->tail = (cb->tail + 1) % cb->max_elements;

    // If tail catches up to head, the buffer is now full
    if (cb->tail == cb->head) {
        cb->is_full = true;
    }

    return true;
}

// Pop an element from the buffer
bool circular_buffer_pop(circular_buffer_t *cb, void *item) {
    if (circular_buffer_is_empty(cb)) {
        return false; // Buffer is empty
    }

    // Copy the item from the head of the buffer
    memcpy(item, (uint8_t *)cb->buffer + (cb->head * cb->element_size), cb->element_size);

    // Move the head to the next position
    cb->head = (cb->head + 1) % cb->max_elements;

    cb->is_full = false; // Popping an element makes the buffer not full

    return true;
}

// Check if the buffer is full
bool circular_buffer_is_full(circular_buffer_t *cb) {
    return cb->is_full;
}

// Check if the buffer is empty
bool circular_buffer_is_empty(circular_buffer_t *cb) {
    return (cb->head == cb->tail && !cb->is_full);
}
