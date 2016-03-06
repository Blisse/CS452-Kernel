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
        init_tracka(&g_track);
    }
    else
    {
        init_trackb(&g_track);
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

TRACK_NODE*
TrackFindNextSensor
    (
        IN TRACK_NODE* node, 
        IN DIRECTION direction
    )
{
    
}
