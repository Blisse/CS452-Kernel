#include <rt.h>
#include <rtosc/buffer.h>
#include <rtosc/assert.h>

#define TEST_BUFFER_SIZE 8

void test_buffer_init() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    T_ASSERT(queue.underlyingBuffer == buffer);
    T_ASSERT(queue.capacity == TEST_BUFFER_SIZE);
    T_ASSERT(queue.front == 0);
    T_ASSERT(queue.back == 0);
}

void test_buffer_add() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    CHAR c = 'C';
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &c, sizeof(c))));

    T_ASSERT(queue.front == 0);
    T_ASSERT(queue.back == sizeof(c));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));

    T_ASSERT(queue.front == 0);
    T_ASSERT(queue.back == sizeof(c) + sizeof(i));
}

void test_buffer_add_full() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 0;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));

    T_ASSERT(RT_FAILURE(RtCircularBufferPush(&queue, &i, sizeof(i))));

    CHAR c = 'c';
    T_ASSERT(RT_FAILURE(RtCircularBufferPush(&queue, &c, sizeof(c))));
}

void test_buffer_add_get_remove() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));

    INT j;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeek(&queue, &j, sizeof(j))));
    T_ASSERT(queue.back == sizeof(i));
    T_ASSERT(i == j);

    INT k;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeekAndPop(&queue, &k, sizeof(k))));
    T_ASSERT(queue.front == queue.back);
    T_ASSERT(i == k);
    T_ASSERT(j == k);
}

void test_buffer_add_overflow() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))));

    CHAR c = 'C';
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &c, sizeof(c))));

    INT j = 2;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &j, sizeof(j))));

    CHAR d;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeekAndPop(&queue, &d, sizeof(d))));
    T_ASSERT(d == c);

    INT k;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPeekAndPop(&queue, &k, sizeof(k))));
    T_ASSERT(j == k);
}

void test_buffer_is_empty_conditions() {
    CHAR buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    INT i = 1;
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == FALSE);
    T_ASSERT(RtCircularBufferIsFull(&queue) == FALSE);
    T_ASSERT(RtCircularBufferSize(&queue) == sizeof(i));

    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))));
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == TRUE);
    T_ASSERT(RtCircularBufferIsFull(&queue) == FALSE);
    T_ASSERT(RtCircularBufferSize(&queue) == 0);

    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == FALSE);
    T_ASSERT(RtCircularBufferIsFull(&queue) == TRUE);
    T_ASSERT(RtCircularBufferSize(&queue) == TEST_BUFFER_SIZE);

    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))));
    T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&queue, sizeof(i))));
    T_ASSERT(RtCircularBufferIsEmpty(&queue) == TRUE);
    T_ASSERT(RtCircularBufferIsFull(&queue) == FALSE);
    T_ASSERT(RtCircularBufferSize(&queue) == 0);
}

void test_buffer_element_at() {
    UINT buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    for (INT i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    }
    T_ASSERT(RT_FAILURE(RtCircularBufferIsFull(&queue)));

    for (INT i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        INT j;
        T_ASSERT(RT_SUCCESS(RtCircularBufferElementAt(&queue, i, &j, sizeof(j))));
        T_ASSERT(i == j);
    }
}

void test_buffer_clear() {
    UINT buffer[TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queue;
    RtCircularBufferInit(&queue, buffer, sizeof(buffer));

    for (INT i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        T_ASSERT(RT_SUCCESS(RtCircularBufferPush(&queue, &i, sizeof(i))));
    }

    T_ASSERT(RT_SUCCESS(RtCircularBufferClear(&queue)));
    T_ASSERT(RtCircularBufferSize(&queue) == 0);
}

int main(int argc, char* argv[]) {

    test_buffer_init();
    test_buffer_add();
    test_buffer_add_full();
    test_buffer_add_get_remove();
    test_buffer_add_overflow();
    test_buffer_is_empty_conditions();
    test_buffer_element_at();
    test_buffer_clear();

    return 0;
}
