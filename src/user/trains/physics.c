#include "physics.h"

#include <rtosc/string.h>
#include <user/trains.h>

static UINT g_steadyStateVelocities[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick
static INT g_steadyStateAcceleration[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick^2
static INT g_steadyStateDeceleration[MAX_TRAINS + 1][MAX_SPEED + 1]; // in micrometers per tick^2

VOID
PhysicsInit()
{
    RtMemset(g_steadyStateVelocities, sizeof(g_steadyStateVelocities), 0);

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateVelocities[i][6] = 3200;
        g_steadyStateVelocities[i][7] = 3552;
        g_steadyStateVelocities[i][8] = 3964;
        g_steadyStateVelocities[i][9] = 4555;
        g_steadyStateVelocities[i][10] = 5082;
        g_steadyStateVelocities[i][11] = 5657;
        g_steadyStateVelocities[i][12] = 6182;
        g_steadyStateVelocities[i][13] = 6633;
        g_steadyStateVelocities[i][14] = 6900;
    }

    RtMemset(g_steadyStateAcceleration, sizeof(g_steadyStateAcceleration), 0);

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateAcceleration[i][5] = 40;
        g_steadyStateAcceleration[i][6] = 40;
        g_steadyStateAcceleration[i][7] = 40;
        g_steadyStateAcceleration[i][8] = 40;
        g_steadyStateAcceleration[i][9] = 40;
        g_steadyStateAcceleration[i][10] = 40;
        g_steadyStateAcceleration[i][11] = 40;
        g_steadyStateAcceleration[i][12] = 40;
        g_steadyStateAcceleration[i][13] = 40;
        g_steadyStateAcceleration[i][14] = 40;
    }

    RtMemset(g_steadyStateDeceleration, sizeof(g_steadyStateDeceleration), 0);

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateDeceleration[i][0] = -40;
        g_steadyStateDeceleration[i][1] = -40;
        g_steadyStateDeceleration[i][2] = -40;
        g_steadyStateDeceleration[i][3] = -40;
        g_steadyStateDeceleration[i][4] = -40;
        g_steadyStateDeceleration[i][5] = -40;
        g_steadyStateDeceleration[i][6] = -40;
        g_steadyStateDeceleration[i][7] = -40;
        g_steadyStateDeceleration[i][8] = -41;
        g_steadyStateDeceleration[i][9] = -41;
        g_steadyStateDeceleration[i][10] = -41;
        g_steadyStateDeceleration[i][11] = -42;
        g_steadyStateDeceleration[i][12] = -44;
        g_steadyStateDeceleration[i][13] = -44;
        g_steadyStateDeceleration[i][14] = -44;
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

