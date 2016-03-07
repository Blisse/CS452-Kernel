#pragma once

#include <rt.h>

VOID
LocationServerCreateTask
    (
        VOID
    );

INT
LocationServerUpdateTrainSpeed
    (
        IN UCHAR train, 
        IN UCHAR speed
    );

INT
LocationServerFlipTrainDirection
    (
        IN UCHAR train
    );
