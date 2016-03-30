#include "track_reserver.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <track/track_data.h>
#include <user/trains.h>

#include "track_server.h"

#define TRACK_RESERVER_NAME "track_reserver"

typedef enum _TRACK_RESERVER_REQUEST_TYPE {
    ReserveRequest = 0,
    ReleaseRequest,
    ClearReservationRequest,
} TRACK_RESERVER_REQUEST_TYPE;

typedef struct _TRACK_RESERVER_RESERVE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
    UINT reserveUntilTick;
} TRACK_RESERVER_RESERVE_REQUEST;

typedef struct _TRACK_RESERVER_RELEASE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
} TRACK_RESERVER_RELEASE_REQUEST;

typedef struct _TRACK_RESERVER_REQUEST {
    TRACK_RESERVER_REQUEST_TYPE type;

    union {
        TRACK_RESERVER_RESERVE_REQUEST reserveRequest;
        TRACK_RESERVER_RELEASE_REQUEST releaseRequest;
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

            case ReserveRequest:
            {
                TRACK_RESERVER_RESERVE_REQUEST* reserveRequest = &request->reserveRequest;
                UINT index;
                VERIFY(SUCCESSFUL(GetIndexOfNode(reserveRequest->reservedNode, &index)));
                break;
            }

            case ReleaseRequest:
            {
                TRACK_RESERVER_RELEASE_REQUEST* releaseRequest = &request->releaseRequest;
                UINT index;
                VERIFY(SUCCESSFUL(GetIndexOfNode(releaseRequest->reservedNode, &index)));
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
    INT trackReserverId = WhoIs(TRACK_RESERVER_NAME);

    BOOLEAN success;
    INT status = Send(trackReserverId, &request, sizeof(request), &success, sizeof(success));
    if (SUCCESSFUL(status))
    {
        return success ? 0 : -1;
    }

    return status;
}

INT
ReserveTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId,
        IN UINT reserveUntilTick
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReserveRequest;
    request.reserveRequest.reservedNode = trackNode;
    request.reserveRequest.trainId = trainId;
    request.reserveRequest.reserveUntilTick = reserveUntilTick;

    return TrackReserverpSendRequest(&request);
}

INT
ReleaseTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReleaseRequest;
    request.releaseRequest.reservedNode = trackNode;
    request.releaseRequest.trainId = trainId;

    return TrackReserverpSendRequest(&request);
}
