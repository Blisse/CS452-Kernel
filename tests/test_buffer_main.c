#include <rt.h>
#include <rtosc/buffer.h>
#include <rtosc/assert.h>

#define TEST_BUFFER_SIZE 8

void test_buffer_init() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    T_ASSERT(queue.underlyingBuffer == buffer, "Wrong buffer assigned.");
    T_ASSERT(queue.capacity == TEST_BUFFER_SIZE, "Wrong buffer size.");
    T_ASSERT(queue.front == 0, "Wrong buffer front.");
    T_ASSERT(queue.back == 0, "Wrong buffer back.");
}

void test_buffer_add() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    CHAR c = 'C';
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &c, sizeof(c))), "Couldn't add to buffer");

    T_ASSERT(queue.front == 0, "Wrong buffer front.");
    T_ASSERT(queue.back == sizeof(c), "Wrong buffer back.");

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");

    T_ASSERT(queue.front == 0, "Wrong buffer front.");
    T_ASSERT(queue.back == sizeof(c) + sizeof(i), "Wrong buffer back.");
}

void test_buffer_add_full() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 0;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");

    T_ASSERT(RT_FAILURE(RtCircularBufferPush(&queue, &i, sizeof(i))), "Shouldn't be able to add to buffer");

    CHAR c = 'c';
    T_ASSERT(RT_FAILURE(RtCircularBufferPush(&queue, &c, sizeof(c))), "Shouldn't be able to add to buffer");
}

void test_buffer_add_get_remove() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");

    INT j;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeek(&queue, &j, sizeof(j))), "Couldn't get from buffer");
    T_ASSERT(queue.back == sizeof(i), "Get should not modify buffer");
    T_ASSERT(i == j, "Got wrong value from buffer");

    INT k;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeekAndPop(&queue, &k, sizeof(k))), "Couldn't remove from buffer");
    T_ASSERT(queue.front == queue.back, "Buffer should be empty");
    T_ASSERT(i == k, "Got wrong value from buffer");
    T_ASSERT(j == k, "Got wrong value from buffer");
}

void test_buffer_add_overflow() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");
    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))), "Couldn't remove from buffer");

    CHAR c = 'C';
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &c, sizeof(c))), "Couldn't add to buffer");

    INT j = 2;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &j, sizeof(j))), "Couldn't add to buffer");

    CHAR d;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeekAndPop(&queue, &d, sizeof(d))), "Couldn't remove from buffer");
    T_ASSERT(d == c, "Got wrong value from buffer");

    INT k;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeekAndPop(&queue, &k, sizeof(k))), "Couldn't remove from buffer");
    T_ASSERT(j == k, "Got wrong value from buffer");
}

void test_buffer_is_empty_conditions() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == FALSE, "Circular buffer is empty");
    T_ASSERT(RtCircularBufferIsFull(&queue) == FALSE, "Circular buffer is full");
    T_ASSERT(RtCircularBufferSize(&queue) == sizeof(i), "Circular buffer wrong size");

    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))), "Couldn't remove from buffer");
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == TRUE, "Circular buffer is not empty");
    T_ASSERT(RtCircularBufferIsFull(&queue) == FALSE, "Circular buffer is full");
    T_ASSERT(RtCircularBufferSize(&queue) == 0, "Circular buffer wrong size");

    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))), "Couldn't add to buffer");
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == FALSE, "Circular buffer is empty");
    T_ASSERT(RtCircularBufferIsFull(&queue) == TRUE, "Circular buffer is not full");
    T_ASSERT(RtCircularBufferSize(&queue) == TEST_BUFFER_SIZE, "Circular buffer wrong size");

    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))), "Couldn't remove from buffer");
    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))), "Couldn't remove from buffer");
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == TRUE, "Circular buffer is not empty");
    T_ASSERT(RtCircularBufferIsFull(&queue) == FALSE, "Circular buffer is full");
    T_ASSERT(RtCircularBufferSize(&queue) == 0, "Circular buffer wrong size");
}

int main(int argc, char* argv[]) {

    test_buffer_init();
    test_buffer_add();
    test_buffer_add_full();
    test_buffer_add_get_remove();
    test_buffer_add_overflow();
    test_buffer_is_empty_conditions();

    return 0;
}
