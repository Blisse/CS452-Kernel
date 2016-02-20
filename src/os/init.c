#include <rtos.h>
#include <rtkernel.h>

#include <user/users.h>

#include <bwio/bwio.h>
#include <rtosc/assert.h>

#include "clock_server.h"
#include "idle.h"
#include "io.h"
#include "name_server.h"
#include "shutdown.h"
#include "uart.h"

VOID
InitOsTasks
    (
        VOID
    )
{
    // Initialize RTOS
    IdleCreateTask();
    NameServerCreateTask();
    ShutdownCreateTask();
    ClockServerCreateTask();
    IoCreateTask();
    UartCreateTasks();

    VERIFY(SUCCESSFUL(Create(HighestUserPriority, InitUserTasks)));
}
