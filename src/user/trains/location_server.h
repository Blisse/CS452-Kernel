#pragma once

#include <rt.h>

VOID
LocationServerCreateTask();

INT
LocationServerUpdateTrainSpeed(
        IN UCHAR train,
        IN UCHAR speed
    );

INT
LocationServerSwitchUpdated();

INT
LocationServerTrainDirectionReverse(
        IN UCHAR train
    );
