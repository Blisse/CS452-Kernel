#include "track_server.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/math.h>
#include <track/track_lib.h>
#include <user/io.h>

#define TRACK_SERVER_NAME "track_server"

typedef enum _TRACK_SERVER_REQUEST_TYPE {
    GetDistanceBetweenNodesRequest = 0,
    GetSensorNodeRequest,
    GetNextSensorNodeRequest,
    GetPathToDestinationRequest,
    GetNextNodesWithinDistanceRequest,
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

typedef struct TRACK_SERVER_PATH_TO_DESTINATION_REQUEST {
    TRACK_NODE* currentNode;
    TRACK_NODE* destinationNode;
    RT_CIRCULAR_BUFFER* path;
} TRACK_SERVER_PATH_TO_DESTINATION_REQUEST;

typedef struct TRACK_SERVER_NEXT_NODES_WITHIN_DISTANCE_REQUEST {
    TRACK_NODE* currentNode;
    UINT distance;
    RT_CIRCULAR_BUFFER* path;
} TRACK_SERVER_NEXT_NODES_WITHIN_DISTANCE_REQUEST;

typedef struct _TRACK_SERVER_REQUEST {
    TRACK_SERVER_REQUEST_TYPE type;

    union {
        TRACK_SERVER_DISTANCE_BETWEEN_NODES_REQUEST distanceBetweenNodesRequest;
        TRACK_SERVER_SENSOR_NODE_REQUEST sensorNodeRequest;
        TRACK_SERVER_NEXT_SENSOR_NODE_REQUEST nextSensorNodeRequest;
        TRACK_SERVER_PATH_TO_DESTINATION_REQUEST pathToDestinationRequest;
        TRACK_SERVER_NEXT_NODES_WITHIN_DISTANCE_REQUEST nextNodesWithinDistanceRequest;
    };
} TRACK_SERVER_REQUEST;

static
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

static
inline
TRACK_NODE*
TrackServerpGetNextNode(
        IN TRACK_NODE* node
    )
{
    return TrackServerpGetNextEdge(node)->dest;
}

static
BOOLEAN
TrackServerpGetNextSensorNode(
        IN TRACK_NODE* currentNode,
        OUT TRACK_NODE** nextSensorNode
    )
{
    TRACK_NODE* nextNode = TrackServerpGetNextNode(currentNode);

    while (nextNode->type != NODE_SENSOR && nextNode->type != NODE_EXIT)
    {
        nextNode = TrackServerpGetNextNode(nextNode);
    }

    if (nextNode->type == NODE_SENSOR)
    {
        *nextSensorNode = nextNode;
        return TRUE;
    }

    return FALSE;
}

static
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
        TRACK_SERVER_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        BOOLEAN requestSuccess = TRUE;

        switch (request.type)
        {
            case GetDistanceBetweenNodesRequest:
            {
                UINT nodeDistance;
                requestSuccess = TrackServerpCalculateDistanceBetweenNodes(
                    request.distanceBetweenNodesRequest.nodeA,
                    request.distanceBetweenNodesRequest.nodeB,
                    &nodeDistance);

                if (requestSuccess)
                {
                    *(request.distanceBetweenNodesRequest.nodeDistance) = nodeDistance;
                }

                break;
            }

            case GetSensorNodeRequest:
            {
                SENSOR* sensor = request.sensorNodeRequest.sensor;
                UINT index = ((sensor->module - 'A') * 16) + (sensor->number - 1);
                TRACK_NODE* sensorNode = &trackNodes[index];
                *(request.sensorNodeRequest.sensorNode) = sensorNode;

                break;
            }

            case GetNextSensorNodeRequest:
            {
                TRACK_NODE* nextSensorNode = NULL;
                requestSuccess = TrackServerpGetNextSensorNode(request.nextSensorNodeRequest.currentNode, &nextSensorNode);

                if (requestSuccess)
                {
                    *(request.nextSensorNodeRequest.nextSensorNode) = nextSensorNode;
                }

                break;
            }

            case GetPathToDestinationRequest:
            {
                TRACK_NODE* currentNode = request.pathToDestinationRequest.currentNode;
                TRACK_NODE* destinationNode = request.pathToDestinationRequest.destinationNode;
                RT_CIRCULAR_BUFFER* path = request.pathToDestinationRequest.path;
                VERIFY(FindPath(trackNodes, sizeof(trackNodes)/sizeof(trackNodes[0]), currentNode, destinationNode, path));
                break;
            }

            case GetNextNodesWithinDistanceRequest:
            {
                TRACK_NODE* currentNode = request.nextNodesWithinDistanceRequest.currentNode;
                UINT distance = request.nextNodesWithinDistanceRequest.distance;
                RT_CIRCULAR_BUFFER* path = request.nextNodesWithinDistanceRequest.path;

                UINT position = 0;
                while (position < distance)
                {
                    TRACK_NODE* iteraterNode;
                    VERIFY(TrackServerpGetNextSensorNode(currentNode, &iteraterNode));
                    VERIFY(RT_SUCCESS(RtCircularBufferPush(path, &currentNode, sizeof(currentNode))));

                    UINT distanceBetweenNodes;
                    VERIFY(TrackServerpCalculateDistanceBetweenNodes(currentNode, iteraterNode, &distanceBetweenNodes));

                    position += distanceBetweenNodes;
                    currentNode = iteraterNode;
                }

                VERIFY(TrackServerpGetNextSensorNode(currentNode, &currentNode));
                VERIFY(RT_SUCCESS(RtCircularBufferPush(path, &currentNode, sizeof(currentNode))));

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
    INT status = Send(trackServerId, request, sizeof(*request), &requestSuccess, sizeof(requestSuccess));

    if (SUCCESSFUL(status))
    {
        return (requestSuccess ? 0 : -1);
    }

    return status;
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

INT
GetPathToDestination(
        IN TRACK_NODE* currentNode,
        IN TRACK_NODE* destinationNode,
        IN RT_CIRCULAR_BUFFER* path
    )
{
    TRACK_SERVER_REQUEST request;
    request.type = GetPathToDestinationRequest;
    request.pathToDestinationRequest.currentNode = currentNode;
    request.pathToDestinationRequest.destinationNode = destinationNode;
    request.pathToDestinationRequest.path = path;

    return TrackServerpSendRequest(&request);
}

INT
GetNextNodesWithinDistance(
        IN TRACK_NODE* currentNode,
        IN UINT distance,
        OUT RT_CIRCULAR_BUFFER* path
    )
{
    TRACK_SERVER_REQUEST request;
    request.type = GetNextNodesWithinDistanceRequest;
    request.nextNodesWithinDistanceRequest.currentNode = currentNode;
    request.nextNodesWithinDistanceRequest.distance = distance;
    request.nextNodesWithinDistanceRequest.path = path;

    return TrackServerpSendRequest(&request);
}
