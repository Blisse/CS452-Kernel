#include "track_reserver.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <track/track_data.h>
#include <user/io.h>
#include <user/trains.h>

#define TRACK_RESERVER_NAME "track_reserver"

typedef enum _TRACK_RESERVER_REQUEST_TYPE {
    ReserveRequest = 0,
    ReserveMultipleRequest,
    IsFreeRequest,
    ReleaseRequest,
    ReleaseAllRequest,
} TRACK_RESERVER_REQUEST_TYPE;

typedef struct _TRACK_RESERVER_RESERVE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
} TRACK_RESERVER_RESERVE_REQUEST;

typedef struct TRACK_RESERVER_RESERVE_MULTIPLE_REQUEST {
    RT_CIRCULAR_BUFFER* reservedNodes;
    UINT trainId;
} TRACK_RESERVER_RESERVE_MULTIPLE_REQUEST;

typedef struct TRACK_RESERVER_IS_FREE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
    BOOLEAN* isFree;
} TRACK_RESERVER_IS_FREE_REQUEST;

typedef struct _TRACK_RESERVER_RELEASE_REQUEST {
    TRACK_NODE* reservedNode;
    UINT trainId;
} TRACK_RESERVER_RELEASE_REQUEST;

typedef struct _TRACK_RESERVER_RELEASE_ALL_REQUEST {
    UINT trainId;
} TRACK_RESERVER_RELEASE_ALL_REQUEST;

typedef struct _TRACK_RESERVER_REQUEST {
    TRACK_RESERVER_REQUEST_TYPE type;

    union {
        TRACK_RESERVER_RESERVE_REQUEST reserveRequest;
        TRACK_RESERVER_RESERVE_MULTIPLE_REQUEST reserveMultipleRequest;
        TRACK_RESERVER_IS_FREE_REQUEST isFreeRequest;
        TRACK_RESERVER_RELEASE_REQUEST releaseRequest;
        TRACK_RESERVER_RELEASE_ALL_REQUEST releaseAllRequest;
    };
} TRACK_RESERVER_REQUEST;

inline
static
BOOLEAN
TrackReserverpIsReservationFree(
        IN INT reservation
    )
{
    return (reservation == -1);
}

inline
static
BOOLEAN
TrackReserverpIsReservationMine(
        IN INT reservation,
        IN INT trainId
    )
{
    return (reservation == trainId);
}

inline
static
BOOLEAN
TrackReserverpIsReservationFreeOrMine(
        IN INT* trackReservations,
        IN UINT trackReservationsSize,
        IN TRACK_NODE* trackNode,
        IN INT trainId
    )
{
    UINT forwardIndex = trackNode->node_index;
    ASSERT(forwardIndex < trackReservationsSize);
    UINT reverseIndex = trackNode->reverse->node_index;
    ASSERT(reverseIndex < trackReservationsSize);

    INT forwardReservation = trackReservations[forwardIndex];
    INT reverseReservation = trackReservations[reverseIndex];

    return ((TrackReserverpIsReservationFree(forwardReservation) || TrackReserverpIsReservationMine(forwardReservation, trainId))
        && (TrackReserverpIsReservationFree(reverseReservation) || TrackReserverpIsReservationMine(reverseReservation, trainId)));
}

inline
static
BOOLEAN
TrackReserverpTryReserve(
        IN INT* trackReservations,
        IN UINT trackReservationsSize,
        IN TRACK_NODE* trackNode,
        IN INT trainId
    )
{
    if (TrackReserverpIsReservationFreeOrMine(trackReservations, trackReservationsSize, trackNode, trainId))
    {
        trackReservations[trackNode->node_index] = trainId;
        trackReservations[trackNode->reverse->node_index] = trainId;
        return TRUE;
    }

    return FALSE;
}

inline
static
BOOLEAN
TrackReserverpTryRelease(
        IN INT* trackReservations,
        IN UINT trackReservationsSize,
        IN TRACK_NODE* trackNode,
        IN INT trainId
    )
{
    if (TrackReserverpIsReservationFreeOrMine(trackReservations, trackReservationsSize, trackNode, trainId))
    {
        trackReservations[trackNode->node_index] = -1;
        trackReservations[trackNode->reverse->node_index] = -1;
        return TRUE;
    }

    return FALSE;
}

VOID
TrackReserverpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(TRACK_RESERVER_NAME)));

    INT trackReservations[TRACK_MAX];
    UINT trackReservationsSize = TRACK_MAX;

    for (UINT i = 0; i < trackReservationsSize; i++)
    {
        trackReservations[i] = -1;
    }

    while (1)
    {
        INT senderId;
        TRACK_RESERVER_REQUEST* request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        BOOLEAN success = TRUE;

        switch (request->type)
        {
            case ReserveRequest:
            {
                TRACK_RESERVER_RESERVE_REQUEST* reserveRequest = &request->reserveRequest;
                success = TrackReserverpTryReserve(trackReservations, trackReservationsSize, reserveRequest->reservedNode, reserveRequest->trainId);
                break;
            }

            case ReserveMultipleRequest:
            {
                TRACK_RESERVER_RESERVE_MULTIPLE_REQUEST* reserveMultipleRequest = &request->reserveMultipleRequest;

                UINT reservedNodesSize = RtCircularBufferSize(reserveMultipleRequest->reservedNodes) / sizeof(TRACK_NODE*);

                for (UINT i = 0; i < reservedNodesSize && success; i++)
                {
                    TRACK_NODE* trackNode;
                    VERIFY(RT_SUCCESS(RtCircularBufferElementAt(reserveMultipleRequest->reservedNodes, i, &trackNode, sizeof(trackNode))));
                    ASSERT(trackNode != NULL);

                    success = TrackReserverpIsReservationFreeOrMine(trackReservations, trackReservationsSize, trackNode, reserveMultipleRequest->trainId);
                }

                if (success)
                {
                    for (UINT i = 0; i < trackReservationsSize; i++)
                    {
                        if (trackReservations[i] == reserveMultipleRequest->trainId)
                        {
                            trackReservations[i] = -1;
                        }
                    }

                    for (UINT i = 0; i < reservedNodesSize && success; i++)
                    {
                        TRACK_NODE* trackNode;
                        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(reserveMultipleRequest->reservedNodes, i, &trackNode, sizeof(trackNode))));
                        ASSERT(trackNode != NULL);

                        success = TrackReserverpTryReserve(trackReservations, trackReservationsSize, trackNode, reserveMultipleRequest->trainId);
                    }
                }

                break;
            }

            case IsFreeRequest:
            {
                TRACK_RESERVER_IS_FREE_REQUEST* isFreeRequest = &request->isFreeRequest;

                *isFreeRequest->isFree = TrackReserverpIsReservationFreeOrMine(trackReservations, trackReservationsSize, isFreeRequest->reservedNode, isFreeRequest->trainId);

                break;
            }

            case ReleaseRequest:
            {
                TRACK_RESERVER_RELEASE_REQUEST* releaseRequest = &request->releaseRequest;

                success = TrackReserverpTryRelease(trackReservations, trackReservationsSize, releaseRequest->reservedNode, releaseRequest->trainId);

                break;
            }

            case ReleaseAllRequest:
            {
                TRACK_RESERVER_RELEASE_ALL_REQUEST* releaseAllRequest = &request->releaseAllRequest;

                for (UINT i = 0; i < trackReservationsSize; i++)
                {
                    if (trackReservations[i] == releaseAllRequest->trainId)
                    {
                        trackReservations[i] = -1;
                    }
                }

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
        IN UINT trainId
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReserveRequest;
    request.reserveRequest.reservedNode = trackNode;
    request.reserveRequest.trainId = trainId;

    return TrackReserverpSendRequest(&request);
}

INT
ReserveTrackMultiple (
        IN RT_CIRCULAR_BUFFER* trackNodes,
        IN UINT trainId
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReserveMultipleRequest;
    request.reserveMultipleRequest.reservedNodes = trackNodes;
    request.reserveMultipleRequest.trainId = trainId;

    return TrackReserverpSendRequest(&request);
}

INT
IsTrackFree (
        IN TRACK_NODE* reservedNode,
        IN UINT trainId,
        OUT BOOLEAN* isFree
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = IsFreeRequest;
    request.isFreeRequest.reservedNode = reservedNode;
    request.isFreeRequest.trainId = trainId;
    request.isFreeRequest.isFree = isFree;

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

INT
ReleaseAllTrack (
        IN UINT trainId
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = ReleaseAllRequest;
    request.releaseAllRequest.trainId = trainId;

    return TrackReserverpSendRequest(&request);
}
