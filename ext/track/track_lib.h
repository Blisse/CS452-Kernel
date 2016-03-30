#pragma once

#include <rt.h>
#include <rtosc/buffer.h>

#include "track_data.h"

BOOLEAN
FindPath(
        IN TRACK_NODE* nodes,
        IN UINT nodesLength,
        IN TRACK_NODE* startNode,
        IN TRACK_NODE* destinationNode,
        IN RT_CIRCULAR_BUFFER* path
    );

UINT
FindPathDistance(
        IN RT_CIRCULAR_BUFFER* path
    );
