#include "physics.h"

#include <rtosc/string.h>
#include <user/trains.h>

static UINT g_steadyStateVelocities[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick

VOID
PhysicsInit
    (
        VOID
    )
{
    RtMemset(g_steadyStateVelocities, sizeof(g_steadyStateVelocities), 0);

    g_steadyStateVelocities[63][5] = 2501;
    g_steadyStateVelocities[63][6] = 3098;
    g_steadyStateVelocities[63][7] = 3717;
    g_steadyStateVelocities[63][8] = 4192;
    g_steadyStateVelocities[63][9] = 4784;
    g_steadyStateVelocities[63][10] = 5361;
    g_steadyStateVelocities[63][11] = 6069;
    g_steadyStateVelocities[63][12] = 6568;
    g_steadyStateVelocities[63][13] = 7092;
    g_steadyStateVelocities[63][14] = 7117;
}

UINT
PhysicsSteadyStateVelocity
    (
        IN UCHAR train, 
        IN UCHAR speed
    )
{
    return g_steadyStateVelocities[train][speed];
}
