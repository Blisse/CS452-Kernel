#include "track_reserver.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <track/track_data.h>
#include <user/trains.h>

#define TRACK_RESERVER_NAME "track_reserver"

typedef enum _TRACK_RESERVER_REQUEST_TYPE {
    ReserveNodeRequest = 0,
    ReleaseNodeRequest,
    ClearReservationRequest,
} TRACK_RESERVER_REQUEST_TYPE;

typedef struct _TRACK_RESERVER_RESERVE_NODE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
    UINT reserveUntilTick;
} TRACK_RESERVER_RESERVE_NODE_REQUEST;

typedef struct _TRACK_RESERVER_RELEASE_NODE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
} TRACK_RESERVER_RELEASE_NODE_REQUEST;

typedef struct _TRACK_RESERVER_REQUEST {
    TRACK_RESERVER_REQUEST_TYPE type;

    union {
        TRACK_RESERVER_RESERVE_NODE_REQUEST reserveNodeRequest;
        TRACK_RESERVER_RELEASE_NODE_REQUEST releaseNodeRequest;
    };
} TRACK_RESERVER_REQUEST;

typedef struct _TRACK_RESERVATION {
    TRACK_NODE* reservedNode;
    UINT trainId;
    UINT reserveUntilTick;
} TRACK_RESERVATION;

VOID
TrackReserverpTask()
{
    TRACK_RESERVATION trackReservations[TRACK_MAX];
    RtMemset(trackReservations, sizeof(trackReservations), 0);

    VERIFY(SUCCESSFUL(RegisterAs(TRACK_RESERVER_NAME)));

    while (1)
    {
        INT senderId;
        TRACK_RESERVER_REQUEST* request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        BOOLEAN success = TRUE;

        switch (request->type)
        {
            case ClearReservationRequest:
            {

                break;
            }

            case ReserveNodeRequest:
            {

                break;
            }

            case ReleaseNodeRequest:
            {
                break;
            }
        }

        VERIFY(SUCCESSFUL(Reply(senderId, &success, sizeof(success))));
    }
}

VOID
TrackReserverCreate()
{
    VERIFY(SUCCESSFUL(Create(Priority19, TrackReserverpTask)));
}

static
inline
INT
TrackReserverpSendRequest(
        IN TRACK_RESERVER_REQUEST* request
    )
{
    INT locationServerId = WhoIs(TRACK_RESERVER_NAME);

    BOOLEAN success;
    INT status = Send(locationServerId, &request, sizeof(request), &success, sizeof(success));
    if (SUCCESSFUL(status))
    {
        return success ? 0 : -1;
    }

    return status;
}

INT
ReserveTrackNode (
        IN TRACK_NODE* trackNode,
        IN UINT trainId,
        IN UINT reserveUntilTick
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReserveNodeRequest;
    request.reserveNodeRequest.reservedNode = trackNode;
    request.reserveNodeRequest.trainId = trainId;
    request.reserveNodeRequest.reserveUntilTick = reserveUntilTick;

    return TrackReserverpSendRequest(&request);
}

INT
ReleaseTrackNode (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReleaseNodeRequest;
    request.releaseNodeRequest.reservedNode = trackNode;
    request.releaseNodeRequest.trainId = trainId;

    return TrackReserverpSendRequest(&request);
}
