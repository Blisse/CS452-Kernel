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
    lowPriorityTask1.priority = LowestPriority;

    lowPriorityTask2.taskId = 2;
    lowPriorityTask2.state = ReadyState;
    lowPriorityTask2.priority = LowestPriority;

    highPriorityTask1.taskId = 3;
    highPriorityTask1.state = ReadyState;
    highPriorityTask1.priority = HighestPriority;

    highPriorityTask2.taskId = 4;
    highPriorityTask2.state = ReadyState;
    highPriorityTask2.priority = HighestPriority;

    bwsetfifo(BWCOM2, OFF);
    bwsetspeed(BWCOM2, 115200);

    SchedulerInit();

    bwprintf(BWCOM2, "Adding tasks \r\n");

    status = SchedulerAddTask(&lowPriorityTask1);
    ASSERT(RT_SUCCESS(status), "Adding lp1 failed \r\n");

    status = SchedulerAddTask(&lowPriorityTask2);
    ASSERT(RT_SUCCESS(status), "Adding lp2 failed \r\n");

    status = SchedulerAddTask(&highPriorityTask1);
    ASSERT(RT_SUCCESS(status), "Adding hp1 failed \r\n");

    status = SchedulerAddTask(&highPriorityTask2);
    ASSERT(RT_SUCCESS(status), "Adding hp2 failed \r\n");

    bwprintf(BWCOM2, "Running scheduler \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting hp1 failed \r\n");
    ASSERT(nextTask->taskId == 3, "Got wrong task: expected hp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 3, "Scheduler has wrong task: expected hp1 \r\n");

    bwprintf(BWCOM2, "Passing hp1 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting hp2 failed \r\n");
    ASSERT(nextTask->taskId == 4, "Got wrong task: expected hp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 4, "Scheduler has wrong task: expected hp2 \r\n");

    bwprintf(BWCOM2, "Passing hp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting hp1 failed \r\n");
    ASSERT(nextTask->taskId == 3, "Got wrong task: expected hp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 3, "Scheduler has wrong task: expected hp1 \r\n");

    bwprintf(BWCOM2, "Exitting hp1 \r\n");
    highPriorityTask1.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting hp2 failed \r\n");
    ASSERT(nextTask->taskId == 4, "Got wrong task: expected hp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 4, "Scheduler has wrong task: expected hp2 \r\n");

    bwprintf(BWCOM2, "Passing hp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting hp2 failed \r\n");
    ASSERT(nextTask->taskId == 4, "Got wrong task: expected hp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 4, "Scheduler has wrong task: expected hp2 \r\n");

    bwprintf(BWCOM2, "Exitting hp2 \r\n");
    highPriorityTask2.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting lp1 failed \r\n");
    ASSERT(nextTask->taskId == 1, "Got wrong task: expected lp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 1, "Scheduler has wrong task: expected hp1 \r\n");

    bwprintf(BWCOM2, "Passing lp1 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting lp2 failed \r\n");
    ASSERT(nextTask->taskId == 2, "Got wrong task: expected lp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 2, "Scheduler has wrong task: expected lp2 \r\n");

    bwprintf(BWCOM2, "Passing lp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting lp1 failed \r\n");
    ASSERT(nextTask->taskId == 1, "Got wrong task: expected lp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 1, "Scheduler has wrong task: expected lp1 \r\n");

    bwprintf(BWCOM2, "Exitting lp1 \r\n");
    lowPriorityTask1.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->taskId);
    ASSERT(RT_SUCCESS(status), "Getting lp2 failed \r\n");
    ASSERT(nextTask->taskId == 2, "Got wrong task: expected lp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->taskId == 2, "Scheduler has wrong task: expected lp2 \r\n");

    bwprintf(BWCOM2, "Exitting lp2 \r\n");
    lowPriorityTask2.state = ZombieState;

    status = SchedulerGetNextTask(&nextTask);
    ASSERT(STATUS_NOT_FOUND == status, "Scheduler should be out of tasks \r\n");

    bwprintf(BWCOM2, "Scheduler exitting \r\n");

    return status;
}
