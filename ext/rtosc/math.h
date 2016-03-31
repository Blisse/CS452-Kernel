#pragma once

#include <rt.h>

inline
INT umToCm(INT um);

inline
INT umToMm(INT um);

inline
INT mmToUm(INT mm);

inline
INT velocity(
        IN INT distance,
        IN INT ticks
    );

inline
INT movingWeightedAverage(
        IN INT newValue,
        IN INT oldValue,
        IN INT newWeight
    );

inline
INT timeToAccelerate(
        IN INT initialVelocity,
        IN INT finalVelocity,
        IN INT acceleration
    );

inline
INT timeToTravelDistance(
        IN INT distance,
        IN INT velocity
    );

INT distanceToAccelerate (
        IN INT initialVelocity,
        IN INT finalVelocity,
        IN INT acceleration
    );
