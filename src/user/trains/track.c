#include <rtosc/assert.h>
#include <track/track_data.h>
#include <user/trains.h>

static TRACK_NODE g_trackNodes[TRACK_MAX];
static TRACK g_track;

VOID
TrackInit
    (
        IN TRACK track
    )
{
    g_track = track;

    if(TrackA == g_track)
    {
        init_tracka(g_trackNodes);
    }
    else
    {
        init_trackb(g_trackNodes);
    }
}

TRACK_NODE*
TrackFindSensor
    (
        IN SENSOR* sensor
    )
{
    UINT index = ((sensor->module - 'A') * 16) + (sensor->number - 1);

    return &g_trackNodes[index];
}

inline
TRACK_EDGE*
TrackNextEdge
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

inline
TRACK_NODE*
TrackNextNode
    (
        IN TRACK_NODE* node
    )
{
    return TrackNextEdge(node)->dest;
}

INT
TrackFindNextSensor
    (
        IN TRACK_NODE* node,
        OUT TRACK_NODE** nextSensor
    )
{
    TRACK_NODE* iterator = TrackNextNode(node);

    while(NODE_SENSOR != iterator->type && NODE_EXIT != iterator->type)
    {
        iterator = TrackNextNode(iterator);
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
    TRACK_EDGE* nextEdge = TrackNextEdge(n1);
    TRACK_NODE* nextNode = nextEdge->dest;
    UINT d = nextEdge->dist;

    while(nextNode != n1 && nextNode != n2 && NODE_EXIT != nextNode->type)
    {
        nextEdge = TrackNextEdge(nextNode);
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

INT
TrackNumBranchesBetween
    (
        IN TRACK_NODE* n1,
        IN TRACK_NODE* n2,
        OUT UINT* numBranches
    )
{
    TRACK_NODE* iterator = TrackNextNode(n1);
    UINT count = 0;

    while(iterator != n1 && iterator != n2 && NODE_EXIT != iterator->type)
    {
        if(NODE_BRANCH == iterator->type || NODE_MERGE == iterator->type)
        {
            count++;
        }

        iterator = TrackNextNode(iterator);
    }

    if(iterator == n2)
    {
        *numBranches = count;
        return 0;
    }
    else
    {
        return -1;
    }
}

UINT
TrackGetCorrectiveTime
    (
        IN TRACK_NODE* node
    )
{
    // Some sensors are sticky and take a little bit longer to trip
    if(NODE_SENSOR == node->type)
    {
        if(TrackB == g_track)
        {
            switch(node->num)
            {
                // C5
                case 36:
                    return 2;

                // C10
                case 41:
                    return 4;

                // C11
                case 42:
                    return 6;

                default:
                    return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
