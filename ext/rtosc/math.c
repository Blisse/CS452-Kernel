#include "math.h"

INT umToCm(INT um) {
    return (um / 10000);
}

INT umToMm(INT um) {
    return (um / 1000);
}

INT mmToUm(INT mm) {
    return (mm * 1000);
}

INT mmToCm(INT mm) {
    return (mm / 10);
}

INT cmToUm(INT cm) {
    return (cm * 10000);
}

INT velocity(
        IN INT distance,
        IN INT ticks
    )
{
    return (distance / ticks);
}

INT distance(
        IN INT velocity,
        IN INT ticks
    )
{
    return (velocity * ticks);
}

INT movingWeightedAverage(
        IN INT newValue,
        IN INT oldValue,
        IN INT weight
    )
{
    return ((weight * newValue) + ((100 - weight) * oldValue)) / 100;
}

INT timeToAccelerate (
        IN INT initialVelocity,
        IN INT finalVelocity,
        IN INT acceleration
    )
{
    return ((finalVelocity - initialVelocity) / acceleration);
}

INT timeToTravelDistance(
        IN INT distance,
        IN INT velocity
    )
{
    return (distance / velocity);
}

INT distanceToAccelerate (
        IN INT initialVelocity,
        IN INT finalVelocity,
        IN INT acceleration
    )
{
    return ((finalVelocity * finalVelocity) - (initialVelocity * initialVelocity) / acceleration);
}
