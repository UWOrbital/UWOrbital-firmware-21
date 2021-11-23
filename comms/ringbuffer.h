#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <FreeRTOS.h>
#include <semphr.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>




#define QUEUE_BYTES 600

// Circular buffer that can contain a fixed number of same size structs.
// Newer pushed items will overwrite older items if the queue is full.
// Usage instructions: view example() in main.c
struct RingBuffer {
    // (Fixed) size of struct.
    uint32_t elem_size;


    // Length of the queue. Each push increments the length by 1
    uint32_t length;

    // Start index of the queue. Each pop increments the start index
    uint32_t start;

    // Locks access to length and start member variables
    SemaphoreHandle_t sem;


    // Lock access to each "slot" in the circular queue
    // todo: optimize this to uint32_t instead
    bool slotlock[QUEUE_BYTES];

    // Storage container
    char buffer[QUEUE_BYTES];
};

// Create and initialize the ringbuffer
struct RingBuffer create_ring_buf(uint32_t elem_size);

// Length of the ringbuffer
uint32_t rb_len(struct RingBuffer* buf);

// Push struct pointed to by `data` to the buf
// Write to position start + length, then increment length by 1
void push_queue(struct RingBuffer *buf, const void *data);


// Try to pop the first element in the queue uint32_to the buffer provided.
// Returns true on success, false on failure
bool pop_queue(struct RingBuffer *buf, char *into);


#endif //RINGBUFFER_H
