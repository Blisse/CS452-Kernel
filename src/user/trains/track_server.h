#pragma once

#include <rt.h>
#include <track/track_data.h>
#include <user/trains.h>
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
GetSensorNode(
    IN SENSOR* sensor,
    OUT TRACK_NODE** sensorNode
    );

INT
GetNextSensorNode(
    IN TRACK_NODE* currentNode,
    OUT TRACK_NODE** nextSensorNode
    );
