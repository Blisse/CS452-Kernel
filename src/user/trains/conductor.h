#pragma once

#include <rt.h>

#include <user/trains.h>

VOID
ConductorServerCreateTask();

INT
ConductorSetTrainSpeed(
        IN INT trainId,
        IN INT trainSpeed
    );

INT
ConductorReverseTrain(
        IN INT trainId,
        IN INT initialTrainSpeed
    );

INT
ConductorSetSwitchDirection(
        IN INT switchId,
        IN SWITCH_DIRECTION switchDirection
    );
