#include <rtosc/assert.h>
#include <track/track_data.h>
#include <user/trains.h>

static TRACK_NODE g_track[TRACK_MAX];

VOID
TrackInit
    (
        IN TRACK track
    )
{
    if(TrackA == track)
    {
        init_tracka(g_track);
    }
    else
    {
        init_trackb(g_track);
    }
}

TRACK_NODE*
TrackFindSensor
    (
        IN SENSOR* sensor
    )
{
    UINT index = ((sensor->module - 'A') * 16) + (sensor->number - 1);

    return &g_track[index];
}

static
inline
TRACK_EDGE*
TrackpNextEdge
    (
        IN TRACK_NODE* node
    )
{
    if(NODE_BRANCH == node->type)
    {
        SWITCH_DIRECTION direction;
        VERIFY(SUCCESSFUL(SwitchGetDirection(node->num, &direction)));

        if(SwitchStraight == direction)
        {
            return &node->edge[DIR_STRAIGHT];
        }
        else
        {
            return &node->edge[DIR_CURVED];
        }
    }
    else
    {
        return &node->edge[DIR_AHEAD];
    }
}

static
inline
TRACK_NODE*
TrackpNextNode
    (
        IN TRACK_NODE* node
    )
{
    return TrackpNextEdge(node)->dest;
}

INT
TrackFindNextSensor
    (
        IN TRACK_NODE* node, 
        OUT TRACK_NODE** nextSensor
    )
{
    TRACK_NODE* iterator = TrackpNextNode(node);

    while(NODE_SENSOR != iterator->type && NODE_EXIT != iterator->type)
    {
        iterator = TrackpNextNode(iterator);
    }

    if(NODE_SENSOR == iterator->type)
    {
        *nextSensor = iterator;
        return 0;
    }
    else
    {
        return -1;
    }
}

INT
TrackDistanceBetween
    (
        IN TRACK_NODE* n1, 
        IN TRACK_NODE* n2, 
        OUT UINT* distance
    )
{
    TRACK_EDGE* nextEdge = TrackpNextEdge(n1);
    TRACK_NODE* nextNode = nextEdge->dest;
    UINT d = nextEdge->dist;

    while(nextNode != n1 && nextNode != n2 && NODE_EXIT != nextNode->type)
    {
        nextEdge = TrackpNextEdge(nextNode);
        nextNode = nextEdge->dest;
        d += nextEdge->dist;
    }

    if(nextNode == n2)
    {
        *distance = d * 1000; // d is in millimeters, need to convert to micrometers
        return 0;
    }
    else
    {
        return -1;
    }
}
