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
