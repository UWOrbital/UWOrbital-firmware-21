#include "ringbuffer.h"
#include <atomic.h>


static uint32_t size_buf(struct RingBuffer *buf) {
    return (int) (sizeof buf->buffer / buf->elem_size);
}

static char *pointer_at(struct RingBuffer *buf, uint32_t index) {
    uint32_t rotated = index % size_buf(buf);
    rotated *= buf->elem_size;
    return buf->buffer + rotated;
}



// Put spin hooks here to check when we're spinning the locks
static void spin_hook() {}

uint32_t rb_len(struct RingBuffer *buf) {
    uint32_t length = buf->length;
    return length;
}

int contention_points = 0;

void push_queue(struct RingBuffer *buf, const void *data) {
    xSemaphoreTake(buf->sem, portMAX_DELAY);
    uint32_t old_length, old_start;
    if (buf->length >= size_buf(buf)) {
        old_length = buf->length;
        old_start = buf->start++;
        buf->start = buf->start % size_buf(buf);
    } else {
        old_length = buf->length++;
        old_start = buf->start;
    }
    uint32_t push_index = (old_length + old_start) % size_buf(buf);
    while (buf->slotlock[push_index]) {
        // Spin lock
        spin_hook();
        contention_points++;
        taskYIELD();
    }
    buf->slotlock[push_index] = true;
    xSemaphoreGive(buf->sem);

    memcpy(pointer_at(buf, push_index), data, buf->elem_size);
    buf->slotlock[push_index] = false;

}

bool pop_queue(struct RingBuffer *buf, char *into) {
    xSemaphoreTake(buf->sem, portMAX_DELAY);
    if (buf->length > 0) {
        buf->length--;
        int old_start = buf->start++ % size_buf(buf);
        while (buf->slotlock[old_start]) {
            // Someone already has this position locked. What to do? Spin until they're done with it.
            spin_hook();
            contention_points++;
            taskYIELD();
        }
        // Lock this position
        buf->slotlock[old_start] = true;

        volatile int protect = *((int *) pointer_at(buf, old_start));
        xSemaphoreGive(buf->sem);
        volatile int protect1 = *((int *) pointer_at(buf, old_start));

        assert(protect == protect1);
        memcpy(into, pointer_at(buf, old_start), buf->elem_size);

        // Unlock this position
        buf->slotlock[old_start] = false;
        return true;
    } else {
        xSemaphoreGive(buf->sem);
        return false;
    }
}


struct RingBuffer create_ring_buf(uint32_t elem_size) {
    struct RingBuffer rb;

    assert(elem_size < QUEUE_BYTES);

    rb.elem_size = elem_size;
    rb.sem = xSemaphoreCreateMutex();
    rb.start = 0;
    rb.length = 0;
    memset(rb.buffer, 0, sizeof(rb.buffer));
    memset(rb.slotlock, 0, sizeof(rb.slotlock));
    return rb;
}
