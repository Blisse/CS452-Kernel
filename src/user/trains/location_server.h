#pragma once

#include <rt.h>

#include <user/trains.h>

VOID
LocationServerCreateTask();

INT
LocationServerUpdateTrainSpeed(
        IN UCHAR trainId,
        IN UCHAR speed
    );

INT
LocationServerSwitchUpdated();

INT
LocationServerTrainDirectionReverse(
        IN UCHAR trainId
    );

INT
LocationServerLookForTrain(
        IN UCHAR trainId,
        IN UCHAR trainSpeed
    );

INT
GetTrainData (
        IN UCHAR train,
        OUT TRAIN_DATA** data
    );
