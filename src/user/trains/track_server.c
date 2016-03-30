#include "track_server.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/math.h>
#include <track/track_data.h>
#include <user/io.h>

#define TRACK_SERVER_NAME "track_server"

typedef enum _TRACK_SERVER_REQUEST_TYPE {
    GetDistanceBetweenNodesRequest = 0,
    GetSensorNodeRequest,
    GetNextSensorNodeRequest,
    GetIndexOfNodeRequest,
} TRACK_SERVER_REQUEST_TYPE;

typedef struct _TRACK_SERVER_DISTANCE_BETWEEN_NODES_REQUEST {
    TRACK_NODE* nodeA;
    TRACK_NODE* nodeB;
    UINT* nodeDistance;
} TRACK_SERVER_DISTANCE_BETWEEN_NODES_REQUEST;

typedef struct _TRACK_SERVER_SENSOR_NODE_REQUEST {
    SENSOR* sensor;
    TRACK_NODE** sensorNode;
} TRACK_SERVER_SENSOR_NODE_REQUEST;

typedef struct _TRACK_SERVER_NEXT_SENSOR_NODE_REQUEST {
    TRACK_NODE* currentNode;
    TRACK_NODE** nextSensorNode;
} TRACK_SERVER_NEXT_SENSOR_NODE_REQUEST;

typedef struct _TRACK_SERVER_INDEX_OF_NODE_REQUEST {
    TRACK_NODE* node;
    UINT* index;
} TRACK_SERVER_INDEX_OF_NODE_REQUEST;

typedef struct _TRACK_SERVER_REQUEST {
    TRACK_SERVER_REQUEST_TYPE type;

    union {
        TRACK_SERVER_INDEX_OF_NODE_REQUEST indexOfNodeRequest;
        TRACK_SERVER_DISTANCE_BETWEEN_NODES_REQUEST distanceBetweenNodesRequest;
        TRACK_SERVER_SENSOR_NODE_REQUEST sensorNodeRequest;
        TRACK_SERVER_NEXT_SENSOR_NODE_REQUEST nextSensorNodeRequest;
    };
} TRACK_SERVER_REQUEST;

inline
TRACK_EDGE*
TrackServerpGetNextEdge(
        IN TRACK_NODE* node
    )
{
    if (NODE_BRANCH == node->type)
    {
        SWITCH_DIRECTION direction;
        VERIFY(SUCCESSFUL(SwitchGetDirection(node->num, &direction)));

        if (SwitchStraight == direction)
        {
            return &node->edge[DIR_STRAIGHT];
        }
        else
        {
            return &node->edge[DIR_CURVED];
        }
    }

    return &node->edge[DIR_AHEAD];
}

inline
TRACK_NODE*
TrackServerpGetNextNode(
        IN TRACK_NODE* node
    )
{
    return TrackServerpGetNextEdge(node)->dest;
}

BOOLEAN
TrackServerpGetNextSensorNode(
        IN TRACK_NODE* currentNode,
        OUT TRACK_NODE** nextSensorNode
    )
{
    TRACK_NODE* nextNode = TrackServerpGetNextNode(currentNode);

    while (nextNode->type != NODE_SENSOR && nextNode->type != NODE_EXIT) {
        nextNode = TrackServerpGetNextNode(nextNode);
    }

    if (nextNode->type == NODE_SENSOR) {
        *nextSensorNode = nextNode;
        return TRUE;
    }

    return FALSE;
}

BOOLEAN
TrackServerpCalculateDistanceBetweenNodes(
        IN TRACK_NODE* nodeA,
        IN TRACK_NODE* nodeB,
        OUT UINT* nodeDistance
    )
{
    TRACK_EDGE* nextEdge = TrackServerpGetNextEdge(nodeA);
    TRACK_NODE* nextNode = nextEdge->dest;
    UINT distance = nextEdge->dist;

    while (nextNode != nodeA && nextNode != nodeB && NODE_EXIT != nextNode->type)
    {
        nextEdge = TrackServerpGetNextEdge(nextNode);
        nextNode = nextEdge->dest;
        distance += nextEdge->dist;
    }

    if (nextNode == nodeB)
    {
        *nodeDistance = mmToUm(distance);
        return TRUE;
    }

    return FALSE;
}

VOID
TrackServerpTask()
{
    TRACK_NODE trackNodes[TRACK_MAX];

    if (CHOSEN_TRACK == TRACK_B)
    {
        init_trackb(trackNodes);
    }
    else
    {
        init_tracka(trackNodes);
    }

    VERIFY(SUCCESSFUL(RegisterAs(TRACK_SERVER_NAME)));

    while (1)
    {
        INT senderId;
        TRACK_SERVER_REQUEST* request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        BOOLEAN requestSuccess = TRUE;

        switch (request->type) {

            case GetDistanceBetweenNodesRequest:
            {
                UINT nodeDistance;
                requestSuccess = TrackServerpCalculateDistanceBetweenNodes(
                    request->distanceBetweenNodesRequest.nodeA,
                    request->distanceBetweenNodesRequest.nodeB,
                    &nodeDistance
                );

                if (requestSuccess) {
                    *(request->distanceBetweenNodesRequest.nodeDistance) = nodeDistance;
                }

                break;
            }

            case GetSensorNodeRequest:
            {
                SENSOR* sensor = request->sensorNodeRequest.sensor;
                UINT index = ((sensor->module - 'A') * 16) + (sensor->number - 1);
                TRACK_NODE* sensorNode = &trackNodes[index];
                *(request->sensorNodeRequest.sensorNode) = sensorNode;

                break;
            }

            case GetNextSensorNodeRequest:
            {
                TRACK_NODE* nextSensorNode;
                requestSuccess = TrackServerpGetNextSensorNode(request->nextSensorNodeRequest.currentNode, &nextSensorNode);

                if (requestSuccess) {
                    *(request->nextSensorNodeRequest.nextSensorNode) = nextSensorNode;
                }

                break;
            }

            case GetIndexOfNodeRequest:
            {
                requestSuccess = FALSE;

                for (UINT i = 0; i < sizeof(trackNodes) && !requestSuccess; i++)
                {
                    if (&trackNodes[i] == request->indexOfNodeRequest.node)
                    {
                        *(request->indexOfNodeRequest.index) = i;
                        requestSuccess = TRUE;
                    }
                }

                break;
            }
        }

        VERIFY(SUCCESSFUL(Reply(senderId, &requestSuccess, sizeof(requestSuccess))));
    }
}

VOID
TrackServerCreate()
{
    VERIFY(SUCCESSFUL(Create(Priority24, TrackServerpTask)));
}

static
inline
INT
TrackServerpSendRequest(
        IN TRACK_SERVER_REQUEST* request
    )
{
    INT trackServerId = WhoIs(TRACK_SERVER_NAME);

    BOOLEAN requestSuccess;
    INT status = Send(trackServerId, &request, sizeof(request), &requestSuccess, sizeof(requestSuccess));

    if (SUCCESSFUL(status))
    {
        return (requestSuccess ? 0 : -1);
    }

    return status;
}

INT
GetIndexOfNode(
    IN TRACK_NODE* node,
    OUT UINT* index
    )
{
    TRACK_SERVER_REQUEST request;
    request.type = GetIndexOfNodeRequest;
    request.indexOfNodeRequest.node = node;
    request.indexOfNodeRequest.index = index;

    return TrackServerpSendRequest(&request);
}

INT
GetDistanceBetweenNodes(
        IN TRACK_NODE* nodeA,
        IN TRACK_NODE* nodeB,
        OUT UINT* distance
    )
{
    TRACK_SERVER_REQUEST request;
    request.type = GetDistanceBetweenNodesRequest;
    request.distanceBetweenNodesRequest.nodeA = nodeA;
    request.distanceBetweenNodesRequest.nodeB = nodeB;
    request.distanceBetweenNodesRequest.nodeDistance = distance;

    return TrackServerpSendRequest(&request);
}

INT
GetSensorNode(
    IN SENSOR* sensor,
    OUT TRACK_NODE** sensorNode
    )
{
    TRACK_SERVER_REQUEST request;
    request.type = GetSensorNodeRequest;
    request.sensorNodeRequest.sensor = sensor;
    request.sensorNodeRequest.sensorNode = sensorNode;

    return TrackServerpSendRequest(&request);
}

INT
GetNextSensorNode(
    IN TRACK_NODE* currentNode,
    OUT TRACK_NODE** nextSensorNode
    )
{
    TRACK_SERVER_REQUEST request;
    request.type = GetNextSensorNodeRequest;
    request.nextSensorNodeRequest.currentNode = currentNode;
    request.nextSensorNodeRequest.nextSensorNode = nextSensorNode;

    return TrackServerpSendRequest(&request);
}
