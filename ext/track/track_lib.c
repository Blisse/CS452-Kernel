#include "track_lib.h"

#include <rtosc/assert.h>

static
INT
TrackNodesIndexOf(
        IN TRACK_NODE* nodes,
        IN UINT nodesLength,
        IN TRACK_NODE* node
    )
{
    for (UINT i = 0; i < nodesLength; i++)
    {
        if (&nodes[i] == node)
        {
            return i;
        }
    }

    return -1;
}

static
VOID
PathVisitNode(
        IN TRACK_NODE* node,
        IN TRACK_NODE* parent,
        IN RT_CIRCULAR_BUFFER* pathQueue
    )
{
    if (node->path_distance == -1)
    {
        node->path_distance = parent->path_distance + 1;
        node->path_parent = parent;
        VERIFY(RT_SUCCESS(RtCircularBufferPush(pathQueue, &node, sizeof(node))));
    }
}

static
VOID
GetPathToDestination(
    IN TRACK_NODE* destinationNode,
    IN RT_CIRCULAR_BUFFER* path
    )
{
    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER foundPath;
    RtCircularBufferInit(&foundPath, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    INT pathSize = destinationNode->path_distance + 1;

    while (destinationNode != NULL)
    {
        VERIFY(RT_SUCCESS(RtCircularBufferPush(&foundPath, &destinationNode, sizeof(destinationNode))));
        destinationNode = destinationNode->path_parent;
    }

    while (pathSize >= 0)
    {
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(&foundPath, pathSize, &destinationNode, sizeof(destinationNode))));
        VERIFY(RT_SUCCESS(RtCircularBufferPush(path, &destinationNode, sizeof(destinationNode))));
        pathSize -= 1;
    }
}

BOOLEAN
FindPath(
        IN TRACK_NODE* nodes,
        IN UINT nodesLength,
        IN TRACK_NODE* startNode,
        IN TRACK_NODE* destinationNode,
        IN RT_CIRCULAR_BUFFER* path
    )
{
    INT startNodeIndex = TrackNodesIndexOf(nodes, nodesLength, startNode);
    ASSERT(startNodeIndex != -1);
    UNREFERENCED_PARAMETER(startNodeIndex);

    INT destinationNodeIndex = TrackNodesIndexOf(nodes, nodesLength, startNode);
    ASSERT(destinationNodeIndex != -1);
    UNREFERENCED_PARAMETER(destinationNodeIndex);

    TRACK_NODE* underlyingQueueBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathQueue;
    RtCircularBufferInit(&pathQueue, underlyingQueueBuffer, sizeof(underlyingQueueBuffer));

    for (UINT i = 0; i < nodesLength; i++)
    {
        nodes[i].path_distance = -1;
        nodes[i].path_parent = NULL;
    }

    startNode->path_distance = 0;
    VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &startNode, sizeof(startNode))));

    while (!RtCircularBufferIsEmpty(&pathQueue))
    {
        TRACK_NODE* currentNode;
        VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&pathQueue, &currentNode, sizeof(currentNode))));

        if (currentNode == destinationNode)
        {
            GetPathToDestination(destinationNode, path);
            return TRUE;
        }

        if (currentNode->type == NODE_BRANCH)
        {
            PathVisitNode(currentNode->edge[DIR_STRAIGHT].dest, currentNode, &pathQueue);
            PathVisitNode(currentNode->edge[DIR_CURVED].dest, currentNode, &pathQueue);
            PathVisitNode(currentNode->reverse, currentNode, &pathQueue);
        }
        else if (currentNode->type == NODE_EXIT)
        {
            PathVisitNode(currentNode->reverse, currentNode, &pathQueue);
        }
        else
        {
            PathVisitNode(currentNode->edge[DIR_AHEAD].dest, currentNode, &pathQueue);
            PathVisitNode(currentNode->reverse, currentNode, &pathQueue);
        }
    }

    return FALSE;
}
