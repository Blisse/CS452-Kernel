#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include "rt.h"
#include "rtos.h"
#include "scheduler.h"
#include "task.h"

INT
main
    (
        VOID
    )
{
    TASK_DESCRIPTOR lowPriorityTask1;
    TASK_DESCRIPTOR lowPriorityTask2;
    TASK_DESCRIPTOR highPriorityTask1;
    TASK_DESCRIPTOR highPriorityTask2;
    TASK_DESCRIPTOR* nextTask;
    RT_STATUS status;

    lowPriorityTask1.taskId = 1;
    lowPriorityTask1.state = ReadyState;
    lowPriorityTask1.priority = LowestUserPriority;

    lowPriorityTask2.taskId = 2;
    lowPriorityTask2.state = ReadyState;
    lowPriorityTask2.priority = LowestUserPriority;

    highPriorityTask1.taskId = 3;
    highPriorityTask1.state = ReadyState;
    highPriorityTask1.priority = HighestUserPriority;

    highPriorityTask2.taskId = 4;
    highPriorityTask2.state = ReadyState;
    highPriorityTask2.priority = HighestUserPriority;

    bwsetfifo(BWCOM2, OFF);
    bwsetspeed(BWCOM2, 115200);

    SchedulerInit();

    bwprintf(BWCOM2, "Adding tasks \r\n");

    status = SchedulerAddTask(&lowPriorityTask1);
    T_ASSERT(RT_SUCCESS(status));

    status = SchedulerAddTask(&lowPriorityTask2);
    T_ASSERT(RT_SUCCESS(status));

    status = SchedulerAddTask(&highPriorityTask1);
    T_ASSERT(RT_SUCCESS(status));

    status = SchedulerAddTask(&highPriorityTask2);
    T_ASSERT(RT_SUCCESS(status));

    bwprintf(BWCOM2, "Running scheduler \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 3);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 3);

    bwprintf(BWCOM2, "Passing hp1 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 4);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 4);

    bwprintf(BWCOM2, "Passing hp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 3);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 3);

    bwprintf(BWCOM2, "Exitting hp1 \r\n");
    highPriorityTask1.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 4);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 4);

    bwprintf(BWCOM2, "Passing hp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 4);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 4);

    bwprintf(BWCOM2, "Exitting hp2 \r\n");
    highPriorityTask2.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 1);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 1);

    bwprintf(BWCOM2, "Passing lp1 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 2);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 2);

    bwprintf(BWCOM2, "Passing lp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 1);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 1);

    bwprintf(BWCOM2, "Exitting lp1 \r\n");
    lowPriorityTask1.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    T_ASSERT(RT_SUCCESS(status));
    T_ASSERT(nextTask->taskId == 2);
    T_ASSERT(SchedulerGetCurrentTask()->taskId == 2);

    bwprintf(BWCOM2, "Exitting lp2 \r\n");
    lowPriorityTask2.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    T_ASSERT(STATUS_NOT_FOUND == status);

    bwprintf(BWCOM2, "Scheduler exitting \r\n");

    return status;
}
