#include "calibration.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <user/trains.h>
#include <user/io.h>

static
VOID
CalibrationpSteadyStateVelocityTask()
{
    Log("Waiting for servers to initialize (8s)");
    VERIFY(SUCCESSFUL(Delay(400)));
    Log("Waiting for servers to initialize (4s)");
    VERIFY(SUCCESSFUL(Delay(400)));

    Log("Calibration sequence started");
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

    Log("Calibration sequence completed in %d ticks", switchCompletionTime - switchTime);
}

VOID
CalibrationCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, CalibrationpSteadyStateVelocityTask)));
}
