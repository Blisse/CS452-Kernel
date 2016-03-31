#pragma once

#include <rt.h>

#include <track/track_data.h>

VOID
TrackReserverCreate();

INT
ReserveTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );

INT
ReleaseTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );
