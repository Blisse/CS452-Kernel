#pragma once

#include <rt.h>
#include <track/track_data.h>
#include <user/trains.h>

VOID
TrackServerCreate();

INT
GetDistanceBetweenNodes(
        IN TRACK_NODE* nodeA,
        IN TRACK_NODE* nodeB,
        OUT UINT* distance
    );

INT
GetNextSensorNode(
        IN TRACK_NODE* currentNode,
        OUT TRACK_NODE** nextSensorNode
    );

INT
GetNextNode(
        IN TRACK_NODE* currentNode,
        OUT TRACK_NODE** nextNode
    );

INT
GetPathToDestination(
        IN TRACK_NODE* currentNode,
        IN TRACK_NODE* destinationNode,
        IN RT_CIRCULAR_BUFFER* path
    );

INT
GetNextNodesWithinDistance(
        IN TRACK_NODE* currentNode,
        IN INT distance,
        OUT RT_CIRCULAR_BUFFER* path
    );
