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
    RtMemset(g_steadyStateAcceleration, sizeof(g_steadyStateAcceleration), 0);
    RtMemset(g_steadyStateDeceleration, sizeof(g_steadyStateDeceleration), 0);

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

    g_steadyStateVelocities[69][7] = 3510;
    g_steadyStateVelocities[69][8] = 3950;
    g_steadyStateVelocities[69][9] = 4400;
    g_steadyStateVelocities[69][10] = 4900;
    g_steadyStateVelocities[69][11] = 5475;
    g_steadyStateVelocities[69][12] = 6000;
    g_steadyStateVelocities[69][13] = 6465;
    g_steadyStateVelocities[69][14] = 6475;

    g_steadyStateVelocities[71][7] = 1680;
    g_steadyStateVelocities[71][8] = 2115;
    g_steadyStateVelocities[71][9] = 2635;
    g_steadyStateVelocities[71][10] = 3220;
    g_steadyStateVelocities[71][11] = 3910;
    g_steadyStateVelocities[71][12] = 4530;
    g_steadyStateVelocities[71][13] = 4820;
    g_steadyStateVelocities[71][14] = 5680;

    g_steadyStateVelocities[58][7] = 1600;
    g_steadyStateVelocities[58][8] = 2150;
    g_steadyStateVelocities[58][9] = 2720;
    g_steadyStateVelocities[58][10] = 3325;
    g_steadyStateVelocities[58][11] = 4080;
    g_steadyStateVelocities[58][12] = 4720;
    g_steadyStateVelocities[58][13] = 5070;
    g_steadyStateVelocities[58][14] = 5975;

    for (UINT i = 0; i < MAX_TRAINS + 1; i++)
    {
        g_steadyStateAcceleration[i][5] = 10;
        g_steadyStateAcceleration[i][6] = 10;
        g_steadyStateAcceleration[i][7] = 10;
        g_steadyStateAcceleration[i][8] = 10;
        g_steadyStateAcceleration[i][9] = 10;
        g_steadyStateAcceleration[i][10] = 10;
        g_steadyStateAcceleration[i][11] = 10;
        g_steadyStateAcceleration[i][12] = 10;
        g_steadyStateAcceleration[i][13] = 10;
        g_steadyStateAcceleration[i][14] = 10;
    }

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

    g_steadyStateDeceleration[71][7] = -19;
    g_steadyStateDeceleration[71][8] = -19;
    g_steadyStateDeceleration[71][9] = -19;
    g_steadyStateDeceleration[71][10] = -19;
    g_steadyStateDeceleration[71][11] = -19;
    g_steadyStateDeceleration[71][12] = -19;
    g_steadyStateDeceleration[71][13] = -19;
    g_steadyStateDeceleration[71][14] = -19;
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

