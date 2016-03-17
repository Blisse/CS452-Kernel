#include "physics.h"

#include <rtosc/string.h>
#include <user/trains.h>

static UINT g_steadyStateVelocities[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick
static INT g_steadyStateAcceleration[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick^2
static INT g_steadyStateDeceleration[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick^2

VOID
PhysicsInit
    (
        VOID
    )
{
    RtMemset(g_steadyStateVelocities, sizeof(g_steadyStateVelocities), 0);

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateVelocities[i][6] = 2958;
        g_steadyStateVelocities[i][7] = 3552;
        g_steadyStateVelocities[i][8] = 3964;
        g_steadyStateVelocities[i][9] = 4555;
        g_steadyStateVelocities[i][10] = 5082;
        g_steadyStateVelocities[i][11] = 5657;
        g_steadyStateVelocities[i][12] = 6182;
        g_steadyStateVelocities[i][13] = 6633;
        g_steadyStateVelocities[i][14] = 6633;
    }

    RtMemset(g_steadyStateAcceleration, sizeof(g_steadyStateAcceleration), 0);

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateAcceleration[i][5] = 5;
        g_steadyStateAcceleration[i][6] = 7;
        g_steadyStateAcceleration[i][7] = 14;
        g_steadyStateAcceleration[i][8] = 17;
        g_steadyStateAcceleration[i][9] = 19;
        g_steadyStateAcceleration[i][10] = 20;
        g_steadyStateAcceleration[i][11] = 20;
        g_steadyStateAcceleration[i][12] = 21;
        g_steadyStateAcceleration[i][13] = 22;
        g_steadyStateAcceleration[i][14] = 22;
    }

    RtMemset(g_steadyStateDeceleration, sizeof(g_steadyStateDeceleration), 0);

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateDeceleration[i][0] = 20;
        g_steadyStateDeceleration[i][1] = 20;
        g_steadyStateDeceleration[i][2] = 20;
        g_steadyStateDeceleration[i][3] = 20;
        g_steadyStateDeceleration[i][4] = 20;
        g_steadyStateDeceleration[i][5] = 20;
        g_steadyStateDeceleration[i][6] = 20;
        g_steadyStateDeceleration[i][7] = 20;
        g_steadyStateDeceleration[i][8] = 21;
        g_steadyStateDeceleration[i][9] = 21;
        g_steadyStateDeceleration[i][10] = 21;
        g_steadyStateDeceleration[i][11] = 22;
        g_steadyStateDeceleration[i][12] = 24;
        g_steadyStateDeceleration[i][13] = 24;
        g_steadyStateDeceleration[i][14] = 24;
    }
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

INT
PhysicsSteadyStateAcceleration
    (
        IN UCHAR train,
        IN UCHAR speed
    )
{
    return g_steadyStateAcceleration[train][speed];
}

INT
PhysicsSteadyStateDeceleration
    (
        IN UCHAR train,
        IN UCHAR speed
    )
{
    return g_steadyStateDeceleration[train][speed];
}

