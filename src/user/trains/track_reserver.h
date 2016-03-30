#pragma once

#include <rt.h>

#include <track/track_data.h>

VOID
TrackReserverCreate();

INT
ReserveTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId,
        IN UINT reserveUntilTick
    );

INT
ReleaseTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );
