#pragma once

#include <rt.h>

inline
INT umToCm(INT um);

inline
INT umToMm(INT um);

inline
INT mmToUm(INT mm);

inline
INT mmToCm(INT mm);

inline
INT cmToUm(INT cm);

inline
INT velocity(
        IN INT distance,
        IN INT ticks
    );

inline
INT distance(
        IN INT velocity,
        IN INT time
    );

inline
INT movingWeightedAverage(
        IN INT newValue,
        IN INT oldValue,
        IN INT weight
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
