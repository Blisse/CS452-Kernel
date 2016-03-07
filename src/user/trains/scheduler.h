#pragma once

#include <rt.h>
#include <track/track_node.h>

VOID
SchedulerCreateTask
    (
        VOID
    );

INT
SchedulerUpdateLocation
    (
        IN TRACK_NODE* currentNode, 
        IN UINT distancePastCurrentNode, 
        IN TRACK_NODE* nextNode, 
        IN UINT velocity
    );
