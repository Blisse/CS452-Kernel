#pragma once

#include <rt.h>

#include <track/track_data.h>

#include <rtosc/buffer.h>

VOID
TrackReserverCreate();

INT
ReserveTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );

INT
ReserveTrackMultiple (
        IN RT_CIRCULAR_BUFFER* trackNodes,
        IN UINT trainId
    );

INT
ReleaseTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );

INT
ReleaseAllTrack (
        IN UINT trainId
    );
