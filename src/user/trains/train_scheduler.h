#pragma once

#include <rt.h>
#include <track/track_node.h>

VOID
SchedulerCreateTask
    (
        VOID
    );

INT
SchedulerTrainChangedNextNode
    (
        IN UCHAR train,
        IN TRACK_NODE* currentNode,
        IN TRACK_NODE* nextNode
    );

INT
SchedulerTrainArrivedAtNextNode
    (
        IN UCHAR train,
        IN INT arrivalTime
    );

INT
SchedulerUpdateLocation
    (
        IN UCHAR train,
        IN UINT distancePastCurrentNode,
        IN UINT velocity
    );
