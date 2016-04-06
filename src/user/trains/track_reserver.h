#pragma once

#include <rt.h>

#include <rtosc/buffer.h>
#include <track/track_data.h>

VOID
TrackReserverCreate();

INT
ReserveTrackMultiple (
        IN RT_CIRCULAR_BUFFER* trackNodes,
        IN UINT trainId
    );

INT
IsTrackFree (
        IN TRACK_NODE* reservedNode,
        IN UINT trainId,
        OUT BOOLEAN* isFree
    );

INT
ReleaseAllTrack (
        IN UINT trainId
    );
