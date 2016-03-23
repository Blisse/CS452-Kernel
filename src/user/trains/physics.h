#pragma once

#include <rt.h>

VOID
PhysicsInit();

UINT
PhysicsSteadyStateVelocity
    (
        IN UCHAR train,
        IN UCHAR speed
    );

INT
PhysicsSteadyStateAcceleration
    (
        IN UCHAR train,
        IN UCHAR speed
    );

INT
PhysicsSteadyStateDeceleration
    (
        IN UCHAR train,
        IN UCHAR speed
    );
