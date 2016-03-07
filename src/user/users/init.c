#include <user/users.h>

#include <rtosc/assert.h>
#include <user/trains.h>
#include <rtkernel.h>
#include <rtos.h>

VOID
InitUserTasks
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, InitTrainTasks)));
}
