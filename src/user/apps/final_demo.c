#include "final_demo.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <user/trains.h>
#include <user/io.h>

#define FINAL_DEMO_SERVER_NAME "final_demo"
static
VOID
FinalDemopTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(FINAL_DEMO_SERVER_NAME)));

    for (INT i = 7; i >= 0; i--)
    {
        Log("Waiting for servers to initialize (%ds)", i);
        VERIFY(SUCCESSFUL(Delay(100)));
    }

    Log("Hello there!");
    Log("Sit tight, I'm initializing the track...");

    UINT switchTime = Time();

#if CHOSEN_TRACK == TRACK_B
    VERIFY(SUCCESSFUL(SwitchSetDirection(18, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(15, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(14, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(11, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(9, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(8, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(6, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(4, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(2, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(1, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(18, SwitchStraight)));
#endif

    UINT switchCompletionTime = Time();

    Log("I finished! It only took %d ticks", switchCompletionTime - switchTime);

    INT senderId;
    VERIFY(SUCCESSFUL(Receive(&senderId, NULL, 0)));
    VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
}

VOID
FinalDemoCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, FinalDemopTask)));
}

VOID
StartDemo()
{
    INT finalDemoTaskId = WhoIs(FINAL_DEMO_SERVER_NAME);
    ASSERT(SUCCESSFUL(finalDemoTaskId));
    VERIFY(SUCCESSFUL(Send(finalDemoTaskId, NULL, 0, NULL, 0)));
}
