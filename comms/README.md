# UWOrbital-firmware-21
The firmware that runs the best 3U cubesat not yet in space



## Ring Queue

`ringbuffer.c` and `ringbuffer.h` contains an implementation for a fixed-size circular queue that can store telemetry structs.
It is thread-safe and can be used across multiple tasks safely. However, it cannot be used in an interrupt yet.

To run this code on the MCU, add `ringbuffer` files to the project and compile.

To run this code on x86, download the FreeRTOS source code directory and place it adjacent (as a sibling directory) to UWOrbital-firmware-21.
Then, run `make test` to run tests, or `make bench` to run benchmarks.

Usage examples and documentation are in file `main.c`, functions `example()` and `example2()`.

### Details

#### Fast path
In the fast path, the critical, mutex-protected code only does only two increments, and one pointer write. 
The potentially time-consuming copying of telemetry data into the queue is not done in the critical section, yet is still thread-safe.

Therefore, even if the telemetry data is very large, and we have multiple slow readers and writers, they can still access
the queue concurrently.

#### Slow path
In the slow path, the critical, mutex-protected code will spin to wait for a slow reader. 

The slow path only happens where there is one slow reader and many fast writers, or one slow writer and many fast readers.

For example, consider a ring buffer that can hold 10 elements. There is one reader who gets pre-empted while copying the struct out of
the queue, then 9 successful writes in a row. 

The slow reader is still copying out of the queue, so the tenth write encounters the slow path and
must wait for the slow reader. This is the slow path,
and obviously, it happens very rarely.

