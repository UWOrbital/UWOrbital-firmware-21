#include "ringbuffer.h"

#include "stdio.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>


#include <stdarg.h>

#include "FreeRTOS.h"
#include "task.h"

struct SampleTelemetry {
    long timestamp;
    float data1;
    float data2;
    char strings[10];
};

struct RingBuffer mm;


// Push/pop operations since program start
int ops = 0;

extern int contention_points;

void push_task() {
    // Simple LFSR PRNG to generate data to push into the queue
    uint32_t hasher = 3906821083;
    for (long i = 1; i < 1000000; i++) {
        uint32_t input = (hasher >> 31) ^ (hasher >> 29) ^ (hasher >> 11) ^ (hasher >> 10);
        hasher <<= 1;
        hasher |= (input & 1);
        struct SampleTelemetry telem = {i, (float) hasher, (float) hasher * 0.823820, "abfgh"};
        push_queue(&mm, (void *) &telem);
        ops++;
        taskYIELD();
    }
    printf("Push exited\n");

    // Bad for a task to exit
    for (;;) {}
}

void recv_task() {
    printf("Start\n");
    TickType_t start = xTaskGetTickCount();
    struct SampleTelemetry received;
    long max_time = 0;
    while (true) {
        if (pop_queue(&mm, (char *) &received)) {
            ops++;
            assert(received.timestamp > max_time);
            max_time = received.timestamp;


            if (received.timestamp % 10000 == 0) {
                float elapsed = (float) (xTaskGetTickCount() - start) / configTICK_RATE_HZ;
                printf("Push/pop ops per sec: %.0f; Contention points: %d\n", ops / elapsed, contention_points);
            }

        } else {
            // Queue is empty
            taskYIELD();
        }
    }
}

static void example();

static void example2();

int main(int argc, char **argv) {
    printf("%s\n", argv[0]);
    if (argc == 2 && strcmp(argv[1], "bench") == 0) {
        printf("Running benchmark\n");
        mm = create_ring_buf(sizeof(struct SampleTelemetry));
        TaskHandle_t pusher, receiver, pusher1, pusher2;
        BaseType_t returned = xTaskCreate(push_task, "Main", 4096, NULL, tskIDLE_PRIORITY + 1, &pusher);
        assert(returned == pdPASS);
        returned = xTaskCreate(recv_task, "Main", 4096, NULL, tskIDLE_PRIORITY + 1, &receiver);
        assert(returned == pdPASS);
        vTaskStartScheduler();
        for (;;) {

        }
    } else {
        printf("Running test\n");
        example();
        example2();
    }
}


static void example() {
    // Create a ringbuffer struct
    struct RingBuffer rb = create_ring_buf(sizeof(struct SampleTelemetry));

    // To push an item, we make the item on the stack first
    struct SampleTelemetry telemetry = {28802, 28.3, 69.420, "abcdef"};

    // Then, cast the item to a const void* pointer, and pass it to push_queue function
    // The length of the item is automatically known when the RingBuffer was created.
    push_queue(&rb, (const void *) &telemetry);

    // Items are pushed via memcpy, so we can push the same telemetry again after modifying it.
    telemetry.timestamp += 5;
    strcpy(telemetry.strings, "ghijkl");
    push_queue(&rb, (const void *) &telemetry);


    // To pop an item off the queue, first allocate enough space on the stack for the item to live.
    struct SampleTelemetry popped; // Does not have to be initialized memory

    // pop_queue returns true on error, false on failure
    assert(pop_queue(&rb, (char *) &popped));

    // pop_queue pops the first item off. That means we should expect these values.
    assert(popped.timestamp == 28802);
    assert(strcmp(popped.strings, "abcdef") == 0);

    struct SampleTelemetry popped2;
    assert(pop_queue(&rb, (char *) &popped2));
    assert(popped2.timestamp == 28807);
    assert(strcmp(popped2.strings, "ghijkl") == 0);

    // We have popped all items off the queue now. The next time we call pop, it should return an error.
    // Must check pop_queue return result! If not, you may be accessing uninitialized memory.
    struct SampleTelemetry popped3;
    assert(pop_queue(&rb, (char *) &popped3) == false);
    // popped3 still contains uninitialized memory...don't touch it.

    printf("Example passed\n");
}


static void example2() {
    struct RingBuffer rb = create_ring_buf(sizeof(struct SampleTelemetry));

    // Push 10000 elements into the queue. Obviously, it cannot hold all items in memory
    for (int i = 0; i <= 10000; i++) {
        struct SampleTelemetry telem = {i, 0, 0, ""};
        push_queue(&rb, (const void *) &telem);
    }

    // Pop elements in the queue until the queue is empty
    struct SampleTelemetry popped;

    // The last timestamp must be monotonically increasing because of the properties of a queue
    int last_timestamp = -1;

    // Push queue returns true on successful pop, false on failed pop (if queue is empty)
    while (pop_queue(&rb, (char *) &popped)) {

        // Since this is a circular queue, we don't know where the timestamp will start. However, we do know
        // timestamps are monotonically increasing.
        if (last_timestamp != -1) {
            assert(popped.timestamp == last_timestamp + 1);
        }
        last_timestamp = popped.timestamp;
    }

    // A circular queue should always store the last x elements pushed, therefore: the last element popped must be the
    // last element pushed.
    assert(last_timestamp == 10000);

    printf("Example2 passed\n");
}