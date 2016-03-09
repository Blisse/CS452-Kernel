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

    g_steadyStateVelocities[63][6] = 2958;
    g_steadyStateVelocities[63][7] = 3552;
    g_steadyStateVelocities[63][8] = 3964;
    g_steadyStateVelocities[63][9] = 4555;
    g_steadyStateVelocities[63][10] = 5082;
    g_steadyStateVelocities[63][11] = 5657;
    g_steadyStateVelocities[63][12] = 6182;
    g_steadyStateVelocities[63][13] = 6633;
    g_steadyStateVelocities[63][14] = 6633;
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
