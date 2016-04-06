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
    TRACK_NODE* node;
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

VOID
TrackReserverpTask()
{
    INT trackReservationByIndex[TRACK_MAX / 2];
    RtMemset(trackReservationByIndex, sizeof(trackReservationByIndex), -1);

    UINT trackReservationSize = sizeof(trackReservationByIndex) / sizeof(trackReservationByIndex[0]);

    VERIFY(SUCCESSFUL(RegisterAs(TRACK_RESERVER_NAME)));

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

                UINT forwardAndReverseIndex = reserveRequest->reservedNode->node_index / 2;

                if (trackReservationByIndex[forwardAndReverseIndex] == -1)
                {
                    trackReservationByIndex[forwardAndReverseIndex] = reserveRequest->trainId;
                }
                else if (trackReservationByIndex[forwardAndReverseIndex] != reserveRequest->trainId)
                {
                    success = FALSE;
                }

                break;
            }

            case ReserveMultipleRequest:
            {
                TRACK_RESERVER_RESERVE_MULTIPLE_REQUEST* reserveMultipleRequest = &request->reserveMultipleRequest;

                UINT reservedNodesSize = RtCircularBufferSize(reserveMultipleRequest->reservedNodes) / sizeof(TRACK_NODE*);

                for (UINT i = 0; i < reservedNodesSize; i++)
                {
                    TRACK_NODE* trackNode;
                    VERIFY(RT_SUCCESS(RtCircularBufferElementAt(reserveMultipleRequest->reservedNodes, i, &trackNode, sizeof(trackNode))));

                    UINT forwardAndReverseIndex = trackNode->node_index / 2;
                    if (trackReservationByIndex[forwardAndReverseIndex] != -1
                        && trackReservationByIndex[forwardAndReverseIndex] != reserveMultipleRequest->trainId)
                    {
                        success = FALSE;
                    }
                }

                if (success)
                {
                    for (UINT i = 0; i < trackReservationSize; i++)
                    {
                        if (trackReservationByIndex[i] == reserveMultipleRequest->trainId)
                        {
                            trackReservationByIndex[i] = -1;
                        }
                    }

                    for (UINT i = 0; i < reservedNodesSize; i++)
                    {
                        TRACK_NODE* trackNode;
                        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(reserveMultipleRequest->reservedNodes, i, &trackNode, sizeof(trackNode))));
                        UINT forwardAndReverseIndex = trackNode->node_index / 2;
                        trackReservationByIndex[forwardAndReverseIndex] = reserveMultipleRequest->trainId;
                    }
                }

                break;
            }

            case IsFreeRequest:
            {
                TRACK_RESERVER_IS_FREE_REQUEST* isFreeRequest = &request->isFreeRequest;

                UINT forwardAndReverseIndex = isFreeRequest->node->node_index / 2;

                *isFreeRequest->isFree = (trackReservationByIndex[forwardAndReverseIndex] != -1
                    || trackReservationByIndex[forwardAndReverseIndex] != isFreeRequest->trainId);

                break;
            }

            case ReleaseRequest:
            {
                TRACK_RESERVER_RELEASE_REQUEST* releaseRequest = &request->releaseRequest;

                UINT forwardAndReverseIndex = releaseRequest->reservedNode->node_index / 2;

                if (trackReservationByIndex[forwardAndReverseIndex] == releaseRequest->trainId)
                {
                    trackReservationByIndex[forwardAndReverseIndex] = -1;
                }
                else
                {
                    success = FALSE;
                }

                break;
            }

            case ReleaseAllRequest:
            {
                TRACK_RESERVER_RELEASE_ALL_REQUEST* releaseAllRequest = &request->releaseAllRequest;

                for (UINT i = 0; i < trackReservationSize; i++)
                {
                    if (trackReservationByIndex[i] == releaseAllRequest->trainId)
                    {
                        trackReservationByIndex[i] = -1;
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
        IN TRACK_NODE* node,
        IN UINT trainId,
        OUT BOOLEAN* isFree
    )
{
    TRACK_RESERVER_REQUEST request;

    request.type = IsFreeRequest;
    request.isFreeRequest.node = node;
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
