#pragma once

#include <rt.h>
#include <track/track_node.h>
#include <user/trains.h>

VOID
SchedulerCreateTask();

INT
UpdateOnSensorNode(
        IN TRAIN_DATA* trainData
    );

INT
UpdateOnTick(
        IN TRAIN_DATA* trainData
    );
