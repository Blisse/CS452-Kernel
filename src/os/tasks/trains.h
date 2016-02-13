#pragma once

#include <rt.h>

/************************************
 *          TRAIN API               *
 ************************************/

INT
TrainSetSpeed
    (
        IN UCHAR train, 
        IN UCHAR speed
    );

INT
TrainReverse
    (
        IN UCHAR train
    );
