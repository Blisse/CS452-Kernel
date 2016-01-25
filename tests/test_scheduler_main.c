#include <bwio/bwio.h>
#include <rtosc/assert.c>
#include "rt.h"
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

    lowPriorityTask1.tid = 1;
    lowPriorityTask1.state = Ready;
    lowPriorityTask1.priority = LowPriority;

    lowPriorityTask2.tid = 2;
    lowPriorityTask2.state = Ready;
    lowPriorityTask2.priority = LowPriority;

    highPriorityTask1.tid = 3;
    highPriorityTask1.state = Ready;
    highPriorityTask1.priority = HighPriority;

    highPriorityTask2.tid = 4;
    highPriorityTask2.state = Ready;
    highPriorityTask2.priority = HighPriority;

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
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting hp1 failed \r\n");
    ASSERT(nextTask->tid == 3, "Got wrong task: expected hp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 3, "Scheduler has wrong task: expected hp1 \r\n");

    bwprintf(BWCOM2, "Passing hp1 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting hp2 failed \r\n");
    ASSERT(nextTask->tid == 4, "Got wrong task: expected hp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 4, "Scheduler has wrong task: expected hp2 \r\n");

    bwprintf(BWCOM2, "Passing hp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting hp1 failed \r\n");
    ASSERT(nextTask->tid == 3, "Got wrong task: expected hp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 3, "Scheduler has wrong task: expected hp1 \r\n");

    bwprintf(BWCOM2, "Exitting hp1 \r\n");
    highPriorityTask1.state = Zombie;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting hp2 failed \r\n");
    ASSERT(nextTask->tid == 4, "Got wrong task: expected hp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 4, "Scheduler has wrong task: expected hp2 \r\n");

    bwprintf(BWCOM2, "Passing hp2 \r\n");
    
    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting hp2 failed \r\n");
    ASSERT(nextTask->tid == 4, "Got wrong task: expected hp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 4, "Scheduler has wrong task: expected hp2 \r\n");

    bwprintf(BWCOM2, "Exitting hp2 \r\n");
    highPriorityTask2.state = Zombie;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting lp1 failed \r\n");
    ASSERT(nextTask->tid == 1, "Got wrong task: expected lp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 1, "Scheduler has wrong task: expected hp1 \r\n");

    bwprintf(BWCOM2, "Passing lp1 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting lp2 failed \r\n");
    ASSERT(nextTask->tid == 2, "Got wrong task: expected lp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 2, "Scheduler has wrong task: expected lp2 \r\n");

    bwprintf(BWCOM2, "Passing lp2 \r\n");

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting lp1 failed \r\n");
    ASSERT(nextTask->tid == 1, "Got wrong task: expected lp1 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 1, "Scheduler has wrong task: expected lp1 \r\n");

    bwprintf(BWCOM2, "Exitting lp1 \r\n");
    lowPriorityTask1.state = Zombie;

    status = SchedulerGetNextTask(&nextTask);
    bwprintf(BWCOM2, "Got task %d \r\n", nextTask->tid);
    ASSERT(RT_SUCCESS(status), "Getting lp2 failed \r\n");
    ASSERT(nextTask->tid == 2, "Got wrong task: expected lp2 \r\n");
    ASSERT(SchedulerGetCurrentTask()->tid == 2, "Scheduler has wrong task: expected lp2 \r\n");

    bwprintf(BWCOM2, "Exitting lp2 \r\n");
    lowPriorityTask2.state = Zombie;

    status = SchedulerGetNextTask(&nextTask);
    ASSERT(STATUS_NOT_FOUND == status, "Scheduler should be out of tasks \r\n");

    bwprintf(BWCOM2, "Scheduler exitting \r\n");

    return status;
}
