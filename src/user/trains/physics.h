#pragma once

#include <rt.h>

VOID
PhysicsInit
    (
        VOID
    );

UINT
PhysicsSteadyStateVelocity
    (
        IN UCHAR train,
        IN UCHAR speed
    );

VOID
PhysicsSetSteadyStateVelocity
    (
        IN UCHAR train,
        IN UCHAR speed,
        IN UINT velocity
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

VOID
PhysicsSetSteadyStateDeceleration
    (
        IN UCHAR train,
        IN UCHAR speed,
        IN INT deceleration
    );
