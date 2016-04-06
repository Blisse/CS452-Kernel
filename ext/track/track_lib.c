#include "track_lib.h"

#include <rtosc/assert.h>

static
VOID
FindPathVisitNode(
        IN TRACK_NODE* node,
        IN TRACK_NODE* parent,
        IN UINT distance,
        IN RT_CIRCULAR_BUFFER* pathQueue
    )
{
    if (node->path_distance == -1)
    {
        node->path_distance = parent->path_distance + distance;
        node->path_parent = parent;
        VERIFY(RT_SUCCESS(RtCircularBufferPush(pathQueue, &node, sizeof(node))));
    }
}

static
VOID
FindPathFromDestination(
        IN TRACK_NODE* destinationNode,
        IN RT_CIRCULAR_BUFFER* path
    )
{
    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER foundPath;
    RtCircularBufferInit(&foundPath, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    while (destinationNode != NULL)
    {
        VERIFY(RT_SUCCESS(RtCircularBufferPush(&foundPath, &destinationNode, sizeof(destinationNode))));
        destinationNode = destinationNode->path_parent;
    }

    for (INT i = (RtCircularBufferSize(&foundPath) / sizeof(TRACK_NODE*)) - 1; i >= 0; i--)
    {
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(&foundPath, i, &destinationNode, sizeof(destinationNode))));
        VERIFY(RT_SUCCESS(RtCircularBufferPush(path, &destinationNode, sizeof(destinationNode))));
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
    ASSERT(&nodes[startNode->node_index] == startNode);
    ASSERT(&nodes[destinationNode->node_index] == destinationNode);

    for (UINT i = 0; i < TRACK_MAX; i++)
    {
        nodes[i].node_index = i;
        nodes[i].path_distance = -1;
        nodes[i].path_parent = 0;
    }

    TRACK_NODE* underlyingQueueBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathQueue;
    RtCircularBufferInit(&pathQueue, underlyingQueueBuffer, sizeof(underlyingQueueBuffer));

    startNode->path_distance = 0;
    VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &startNode, sizeof(startNode))));

    UINT i = 0;

    while (!RtCircularBufferIsEmpty(&pathQueue) && i++ < TRACK_MAX)
    {
        TRACK_NODE* currentNode;
        VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&pathQueue, &currentNode, sizeof(currentNode))));

        if (currentNode == destinationNode)
        {
            FindPathFromDestination(destinationNode, path);
            return TRUE;
        }
        else if (currentNode->reverse == destinationNode)
        {
            FindPathFromDestination(currentNode, path);
            return TRUE;
        }

        if (currentNode->type == NODE_BRANCH)
        {
            FindPathVisitNode(currentNode->edge[DIR_STRAIGHT].dest, currentNode, currentNode->edge[DIR_STRAIGHT].dist, &pathQueue);
            FindPathVisitNode(currentNode->edge[DIR_CURVED].dest, currentNode, currentNode->edge[DIR_CURVED].dist, &pathQueue);
            // FindPathVisitNode(currentNode->reverse, currentNode, 1, &pathQueue);
        }
        else if (currentNode->type == NODE_EXIT)
        {
            // FindPathVisitNode(currentNode->reverse, currentNode, 1, &pathQueue);
        }
        else
        {
            FindPathVisitNode(currentNode->edge[DIR_AHEAD].dest, currentNode, currentNode->edge[DIR_AHEAD].dist, &pathQueue);
            // FindPathVisitNode(currentNode->reverse, currentNode, 1, &pathQueue);
        }
    }

    return FALSE;
}
