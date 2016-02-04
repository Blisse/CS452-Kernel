#include <rtosc/priority_queue.h>

#include <rtosc/assert.h>

#define TEST_BUFFER_SIZE 8
#define TEST_NUM_PRIORITIES 5

void test_priority_queue_init() {
    INT buffers[TEST_NUM_PRIORITIES][TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queues[TEST_NUM_PRIORITIES];
    RT_PRIORITY_QUEUE priorityQueue;
    RtPriorityQueueInit(&priorityQueue, buffers, queues, sizeof(INT), TEST_NUM_PRIORITIES, TEST_BUFFER_SIZE);

    T_ASSERT(priorityQueue.buffers == buffers, "Wrong buffer assigned.");
    T_ASSERT(priorityQueue.queues == queues, "Wrong queues assigned.");
    T_ASSERT(priorityQueue.bitmask == 0, "Wrong bitmask assigned.");
}

void test_priority_queue_push() {
    INT buffers[TEST_NUM_PRIORITIES][TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queues[TEST_NUM_PRIORITIES];
    RT_PRIORITY_QUEUE priorityQueue;
    RtPriorityQueueInit(&priorityQueue, buffers, queues, sizeof(INT), TEST_NUM_PRIORITIES, TEST_BUFFER_SIZE);

    INT i = 5;
    T_ASSERT(RT_SUCCESS(RtPriorityQueuePush(&priorityQueue, 1, &i, sizeof(i))), "Could not push to priority queue");
    T_ASSERT(priorityQueue.bitmask != 0, "Bitmask still empty");

    INT j;
    T_ASSERT(RT_SUCCESS(RtPriorityQueuePeek(&priorityQueue, &j, sizeof(j))), "Could not peek from priority queue");

    T_ASSERT(i == j, "Push and peek values differ.");
}

void test_priority_queue_pop() {
    INT buffers[TEST_NUM_PRIORITIES][TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queues[TEST_NUM_PRIORITIES];
    RT_PRIORITY_QUEUE priorityQueue;
    RtPriorityQueueInit(&priorityQueue, buffers, queues, sizeof(INT), TEST_NUM_PRIORITIES, TEST_BUFFER_SIZE);

    INT i = 5;
    T_ASSERT(RT_SUCCESS(RtPriorityQueuePush(&priorityQueue, 1, &i, sizeof(i))), "Could not push to priority queue");
    T_ASSERT(RT_SUCCESS(RtPriorityQueuePop(&priorityQueue, sizeof(i))), "Could not pop from priority queue");

    T_ASSERT(priorityQueue.bitmask == 0, "Bitmask not empty");

    INT j;
    T_ASSERT(RT_FAILURE(RtPriorityQueuePeek(&priorityQueue, &j, sizeof(j))), "Should not be able to peek from priority queue");
}

void test_priority_queue_push_multiple() {
    INT buffers[TEST_NUM_PRIORITIES][TEST_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER queues[TEST_NUM_PRIORITIES];
    RT_PRIORITY_QUEUE priorityQueue;
    RtPriorityQueueInit(&priorityQueue, buffers, queues, sizeof(INT), TEST_NUM_PRIORITIES, TEST_BUFFER_SIZE);

    INT i;
    for (i = 10; i < 13; i++)
    {
        T_ASSERT(RT_SUCCESS(RtPriorityQueuePush(&priorityQueue, 1, &i, sizeof(i))), "Could not push to priority queue");
    }
    for (i = 0; i < 4; i++)
    {
        T_ASSERT(RT_SUCCESS(RtPriorityQueuePush(&priorityQueue, 2, &i, sizeof(i))), "Could not push to priority queue");
    }
    for (i = 0; i < 4; i++)
    {
        T_ASSERT(RT_SUCCESS(RtPriorityQueuePush(&priorityQueue, 2, &i, sizeof(i))), "Should be unable to push to priority queue");
    }

    T_ASSERT(RT_FAILURE(RtPriorityQueuePush(&priorityQueue, 2, &i, sizeof(i))), "Should be unable to push to priority queue");

    INT j;
    for (i = 0; i < 4; i++)
    {
        T_ASSERT(RT_SUCCESS(RtPriorityQueuePeekAndPop(&priorityQueue, &j, sizeof(j))), "Could not peek and pop from priority queue");
        T_ASSERT(j == i, "Push and peek values differ.");
    }
    for (i = 0; i < 4; i++)
    {
        T_ASSERT(RT_SUCCESS(RtPriorityQueuePeekAndPop(&priorityQueue, &j, sizeof(j))), "Could not peek and pop from priority queue");
        T_ASSERT(j == i, "Push and peek values differ.");
    }
    for (i = 10; i < 13; i++)
    {
        T_ASSERT(RT_SUCCESS(RtPriorityQueuePeekAndPop(&priorityQueue, &j, sizeof(j))), "Could not peek and pop from priority queue");
        T_ASSERT(j == i, "Push and peek values differ.");
    }
}

int main(int argc, char const *argv[])
{
    test_priority_queue_init();
    test_priority_queue_push();
    test_priority_queue_pop();
    test_priority_queue_push_multiple();

    return 0;
}
