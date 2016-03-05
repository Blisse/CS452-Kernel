#include "track_server.h"

#include <track/track_data.h>

#define TRACK_SERVER_NAME "track"

typedef struct _TRACK_REQUEST_TYPE
{
    ShutdownRequest = 0,
    GetNextSensorRequest,
} TRACK_REQUEST_TYPE;

typedef struct _TRACK_REQUEST
{
    TRACK_REQUEST_TYPE type;
} TRACK_REQUEST;

VOID
TrackServerpSendMessage
    (
        TRACK_REQUEST request
    )
{
    INT result = WhoIs(TRACK_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        INT trackServerId = result;
        result = Send(trackServerId, &request, sizeof(request), NULL, 0);
    }

    return result;
}

static
VOID
TrackServerpShutdownHook
    (
        VOID
    )
{
    TRACK_REQUEST request = { ShutdownRequest };
    VERIFY(SUCCESSFUL(TrackServerpSendMessage(request)));
}

static
VOID
TrackServerpGetNextSensor
    (
        SENSOR_DATA* sensorData
    )
{

}

static
VOID
TrackServerpTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(TRACK_SERVER_NAME)));
    VERIFY(SUCCESSFUL(ShutdownRegisterHook(TrackServerpShutdownHook)));

    track_node tracka[TRACK_MAX];
    init_tracka(tracka);

    track_node trackb[TRACK_MAX];
    init_trackb(trackb);

    track_node* track = tracka;

    BOOLEAN running = TRUE;
    while(running)
    {
        INT sender;
        TRACK_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case GetNextSensorRequest:
            {
                break;
            }
            case ShutdownRequest:
            {
                running = FALSE;
                break;
            }
        }

        VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));
    }
}

VOID
TrackServerCreate
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority16, TrackServerpTask)));
}

VOID
NextSensor
    (
        SENSOR* currentSensor
    )
{
    TRACK_REQUEST request = { GetNextSensorRequest };
    VERIFY(SUCCESSFUL(TrackServerpSendMessage(request)));
}
