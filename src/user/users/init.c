#include <user/users.h>

#include <rtosc/assert.h>
#include <user/apps.h>
#include <user/io.h>
#include <user/trains.h>
#include <rtkernel.h>

VOID
InitUserTasks()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, InitIoTasks)));
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, InitTrainTasks)));
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, InitAppsTasks)));
}
