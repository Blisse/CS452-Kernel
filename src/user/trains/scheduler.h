#pragma once

#include <rt.h>
#include <track/track_node.h>
#include <user/trains.h>

VOID
SchedulerCreateTask();

INT
SchedulerUpdateTrainData(
        IN TRAIN_DATA* trainData
    );
