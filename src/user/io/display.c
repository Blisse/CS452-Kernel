#include "display.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/bitset.h>
#include <rtosc/buffer.h>
#include <rtosc/math.h>
#include <rtosc/string.h>

#include <user/trains.h>
#include <user/io.h>

#define CURSOR_MOVE         "\033[%d;%dH"
#define CURSOR_CLEAR        "\033[2J"
#define CURSOR_DELETE_LINE  "\033[K"
#define CURSOR_HIDE         "\033[?25l"

#define CURSOR_RESET    "\033[0;0m"

#define CURSOR_RED      "\033[31m"
#define CURSOR_GREEN    "\033[32m"
#define CURSOR_YELLOW   "\033[33m"
#define CURSOR_BLUE     "\033[34m"
#define CURSOR_MAGENTA  "\033[35m"
#define CURSOR_CYAN     "\033[36m"
#define CURSOR_WHITE    "\033[0;37m"
#define CURSOR_BOLD_WHITE   "\033[1;37m"

#define DISPLAY_NAME "display"

typedef enum _DISPLAY_REQUEST_TYPE {
    DisplayCharRequest = 0,
    DisplayClockRequest,
    DisplayIdleRequest,
    DisplayLogRequest,
    DisplaySensorRequest,
    DisplayShutdownRequest,
    DisplaySwitchRequest,
    DisplayTrainArrivalRequest,
    DisplayTrainLocationRequest,
} DISPLAY_REQUEST_TYPE;

typedef struct _DISPLAY_LOG_REQUEST {
    CHAR message[60];
    INT length;
} DISPLAY_LOG_REQUEST;

typedef struct _DISPLAY_SENSOR_REQUEST {
    SENSOR_DATA data;
} DISPLAY_SENSOR_REQUEST;

typedef struct _DISPLAY_SWITCH_REQUEST {
    INT index;
    INT number;
    SWITCH_DIRECTION direction;
} DISPLAY_SWITCH_REQUEST;

typedef struct _DISPLAY_TRAIN_ARRIVAL_REQUEST {
    UINT trainId;
    TRACK_NODE* node;
    INT tick;
    INT distance;
} DISPLAY_TRAIN_ARRIVAL_REQUEST;

typedef struct _DISPLAY_TRAIN_LOCATION_REQUEST {
    TRAIN_DATA* trainData;
} DISPLAY_TRAIN_LOCATION_REQUEST;

typedef struct _DISPLAY_REQUEST {
    DISPLAY_REQUEST_TYPE type;

    union {
        CHAR commandLineChar;
        INT clockTicks;
        INT idlePercentage;
        DISPLAY_LOG_REQUEST* logRequest;
        DISPLAY_SENSOR_REQUEST sensorRequest;
        DISPLAY_SWITCH_REQUEST switchRequest;
        DISPLAY_TRAIN_ARRIVAL_REQUEST arrivalRequest;
        DISPLAY_TRAIN_LOCATION_REQUEST locationRequest;
    };

} DISPLAY_REQUEST;

#define CURSOR_CMD_X 20
#define CURSOR_CMD_Y 2

#define CURSOR_CLOCK_X 10
#define CURSOR_CLOCK_Y 2

#define CURSOR_IDLE_X 4
#define CURSOR_IDLE_Y 2

#define CURSOR_LOG_X 4
#define CURSOR_LOG_Y 29

#define CURSOR_SWITCH_X 4
#define CURSOR_SWITCH_Y 5

#define CURSOR_SENSOR_X 16
#define CURSOR_SENSOR_Y 6

#define CURSOR_TRAIN_ARRIVAL_X 20
#define CURSOR_TRAIN_ARRIVAL_Y 10

#define CURSOR_TRAIN_LOCATION_X 20
#define CURSOR_TRAIN_LOCATION_Y 4

typedef struct _CURSOR_POSITION {
    INT x;
    INT y;
} CURSOR_POSITION;

static
inline
INT
WriteCursorPosition (
        IN IO_DEVICE* com2Device,
        IN CURSOR_POSITION* cursor
    )
{
    return WriteFormattedString(com2Device, CURSOR_MOVE, cursor->y, cursor->x);
}

static
VOID
DisplaypCommandLine (
        IN IO_DEVICE* com2Device,
        IN CURSOR_POSITION* cursor,
        IN CHAR c
    )
{
    switch (c)
    {
        case '\r':
        {
            cursor->x = CURSOR_CMD_X;
            WriteCursorPosition(com2Device, cursor);
            WriteString(com2Device, CURSOR_DELETE_LINE);
            break;
        }
        case '\b':
        {
            if (cursor->x > CURSOR_CMD_X)
            {
                cursor->x--;
                WriteCursorPosition(com2Device, cursor);
                WriteString(com2Device, CURSOR_DELETE_LINE);
            }
            break;
        }
        default:
        {
            WriteCursorPosition(com2Device, cursor);
            WriteChar(com2Device, c);
            cursor->x++;
            break;
        }
    }
}

static
VOID
DisplaypClock (
        IN IO_DEVICE* com2Device,
        IN INT ticks
    )
{
    INT ts = (ticks / 10);
    INT s = (ts / 10);
    INT m = (s / 60);

    CURSOR_POSITION cursor = { CURSOR_CLOCK_X, CURSOR_CLOCK_Y };
    WriteCursorPosition(com2Device, &cursor);
    WriteFormattedString(com2Device, "|%02d:%02d:%d>", m % 60, s % 60, ts % 10);
}

static
VOID
DisplaypIdlePercentage (
        IN IO_DEVICE* com2Device,
        IN INT idlePercentage
    )
{
    CURSOR_POSITION cursor = { CURSOR_IDLE_X, CURSOR_IDLE_Y };
    WriteCursorPosition(com2Device, &cursor);
    WriteFormattedString(com2Device, "%02d.%02d%%", idlePercentage / 100, idlePercentage % 100);
}

static
VOID
DisplaypLogRequest (
        IN IO_DEVICE* com2Device,
        IN RT_CIRCULAR_BUFFER* logBuffer,
        IN DISPLAY_LOG_REQUEST* logRequest
    )
{
    if (RtCircularBufferIsFull(logBuffer))
    {
        RtCircularBufferPop(logBuffer, sizeof(logRequest->message));
    }

    RtCircularBufferPush(logBuffer, &logRequest->message, sizeof(logRequest->message));

    UINT logBufferSize = RtCircularBufferSize(logBuffer) / sizeof(logRequest->message);

    for (UINT i = 0; i < logBufferSize; i++)
    {
        CHAR buffer[60];
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(logBuffer, i, buffer, sizeof(buffer))));

        CURSOR_POSITION cursor = { CURSOR_LOG_X, CURSOR_LOG_Y + i };
        VERIFY(SUCCESSFUL(WriteCursorPosition(com2Device, &cursor)));
        VERIFY(SUCCESSFUL(WriteFormattedString(com2Device, CURSOR_DELETE_LINE "%s", buffer)));
    }
}

static
VOID
DisplaypSwitchRequest (
        IN IO_DEVICE* com2Device,
        IN DISPLAY_SWITCH_REQUEST* switchRequest
    )
{
    CURSOR_POSITION cursor = { CURSOR_SWITCH_X, CURSOR_SWITCH_Y + switchRequest->index };
    WriteCursorPosition(com2Device, &cursor);
    WriteFormattedString(com2Device, "sw %3d " CURSOR_YELLOW "%c" CURSOR_RESET, switchRequest->number, switchRequest->direction == SwitchStraight ? 'S' : 'C');
}

static
VOID
DisplaypSensorRequest (
        IN IO_DEVICE* com2Device,
        IN DISPLAY_SENSOR_REQUEST* sensorRequest,
        IN RT_CIRCULAR_BUFFER* sensorDataBuffer
    )
{
    SENSOR_DATA sensorData = sensorRequest->data;
    if (RtCircularBufferIsFull(sensorDataBuffer))
    {
        RtCircularBufferPop(sensorDataBuffer, sizeof(sensorData));
    }
    RtCircularBufferPush(sensorDataBuffer, &sensorData, sizeof(sensorData));

    UINT sensorDataBufferSize = RtCircularBufferSize(sensorDataBuffer) / sizeof(sensorData);
    for (UINT i = 0; i < sensorDataBufferSize; i++)
    {
        SENSOR_DATA displayData;
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(sensorDataBuffer, i, &displayData, sizeof(displayData))));

        CURSOR_POSITION cursor = { CURSOR_SENSOR_X, CURSOR_SENSOR_Y + i };
        VERIFY(SUCCESSFUL(WriteCursorPosition(com2Device, &cursor)));
        VERIFY(SUCCESSFUL(WriteFormattedString(com2Device,
                                               "%c%02d " CURSOR_YELLOW "%d" CURSOR_RESET,
                                               displayData.sensor.module,
                                               displayData.sensor.number,
                                               displayData.isOn)));
    }
}

static
VOID
DisplaypTrainArrivalRequest (
        IN IO_DEVICE* com2Device,
        IN RT_CIRCULAR_BUFFER* arrivalRequestBuffer,
        IN DISPLAY_TRAIN_ARRIVAL_REQUEST* arrivalRequest
    )
{

    if (RtCircularBufferIsFull(arrivalRequestBuffer))
    {
        RtCircularBufferPop(arrivalRequestBuffer, sizeof(DISPLAY_TRAIN_ARRIVAL_REQUEST));
    }

    RtCircularBufferPush(arrivalRequestBuffer, arrivalRequest, sizeof(DISPLAY_TRAIN_ARRIVAL_REQUEST));

    UINT arrivalRequestBufferSize = RtCircularBufferSize(arrivalRequestBuffer) / sizeof(DISPLAY_TRAIN_ARRIVAL_REQUEST);

    for (UINT i = 0; i < arrivalRequestBufferSize; i++)
    {
        DISPLAY_TRAIN_ARRIVAL_REQUEST request;
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(arrivalRequestBuffer, i, &request, sizeof(request))));

        CURSOR_POSITION cursor = { CURSOR_TRAIN_ARRIVAL_X, CURSOR_TRAIN_ARRIVAL_Y + i };
        VERIFY(SUCCESSFUL(WriteCursorPosition(com2Device, &cursor)));
        VERIFY(SUCCESSFUL(WriteFormattedString(com2Device, CURSOR_DELETE_LINE "%d| @ %s, off by %d ticks, %d cm",
            request.trainId,
            request.node->name,
            request.tick,
            umToCm(request.distance))));
    }
}

static
VOID
DisplaypTrainLocationRequest (
        IN IO_DEVICE* com2Device,
        IN INT trainIndex,
        IN DISPLAY_TRAIN_LOCATION_REQUEST* locationRequest
    )
{
    TRAIN_DATA* trainData = locationRequest->trainData;

    CURSOR_POSITION cursor = { CURSOR_TRAIN_LOCATION_X, CURSOR_TRAIN_LOCATION_Y + trainIndex };
    VERIFY(SUCCESSFUL(WriteCursorPosition(com2Device, &cursor)));
    VERIFY(SUCCESSFUL(WriteFormattedString(com2Device, CURSOR_BOLD_WHITE "%d| @ %s->%s \t by %4d cm, %5d u/t, %2d u/t^2" CURSOR_RESET,
                                           trainData->trainId,
                                           trainData->currentNode->name,
                                           trainData->nextNode->name,
                                           umToCm(trainData->distancePastCurrentNode),
                                           trainData->velocity,
                                           trainData->acceleration)));
}

static
INT
DisplaypSendRequest (
        IN DISPLAY_REQUEST* request
    )
{
    INT displayServerId = WhoIs(DISPLAY_NAME);
    ASSERT(SUCCESSFUL(displayServerId));
    return Send(displayServerId, request, sizeof(*request), NULL, 0);
}

static
VOID
DisplaypShutdownHook()
{
    DISPLAY_REQUEST request;
    request.type = DisplayShutdownRequest;
    DisplaypSendRequest(&request);
}

static
VOID
DisplaypTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(DISPLAY_NAME)));
    VERIFY(SUCCESSFUL(ShutdownRegisterHook(DisplaypShutdownHook)));

    IO_DEVICE com2Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2Device)));

    WriteString(&com2Device, CURSOR_HIDE);
    WriteString(&com2Device, CURSOR_CLEAR);

    {
        CURSOR_POSITION cursor;
        cursor.x = 4;
        cursor.y = 4;
        WriteCursorPosition(&com2Device, &cursor);
        WriteString(&com2Device, "---------------");

        for (UINT i = 5; i < 27; i++)
        {
            cursor.y = i;
            WriteCursorPosition(&com2Device, &cursor);
            WriteString(&com2Device, "         |-----");
        }

        cursor.y = 27;
        WriteCursorPosition(&com2Device, &cursor);
        WriteString(&com2Device, "---------------");
    }

    SENSOR_DATA underlyingSensorDataBuffer[8];
    RT_CIRCULAR_BUFFER sensorDataBuffer;
    RtCircularBufferInit(&sensorDataBuffer, underlyingSensorDataBuffer, sizeof(underlyingSensorDataBuffer));

    CHAR underlyingLogBuffer[600];
    RT_CIRCULAR_BUFFER logBuffer;
    RtCircularBufferInit(&logBuffer, underlyingLogBuffer, sizeof(underlyingLogBuffer));

    DISPLAY_TRAIN_ARRIVAL_REQUEST underlyingArrivalRequestBuffer[4];
    RT_CIRCULAR_BUFFER arrivalRequestBuffer;
    RtCircularBufferInit(&arrivalRequestBuffer, underlyingArrivalRequestBuffer, sizeof(underlyingArrivalRequestBuffer));

    INT trainLocationIndex[4];
    RtMemset(trainLocationIndex, sizeof(trainLocationIndex), 0);

    CURSOR_POSITION cursor = { CURSOR_CMD_X, CURSOR_CMD_Y };

    BOOLEAN running = TRUE;
    while (running)
    {
        INT senderId;
        DISPLAY_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch (request.type)
        {
            case DisplayCharRequest:
            {
                DisplaypCommandLine(&com2Device, &cursor, request.commandLineChar);
                break;
            }
            case DisplayClockRequest:
            {
                DisplaypClock(&com2Device, request.clockTicks);
                break;
            }
            case DisplayIdleRequest:
            {
                DisplaypIdlePercentage(&com2Device, request.idlePercentage);
                break;
            }
            case DisplayLogRequest:
            {
                DisplaypLogRequest(&com2Device, &logBuffer, request.logRequest);
                break;
            }
            case DisplaySwitchRequest:
            {
                DisplaypSwitchRequest(&com2Device, &request.switchRequest);
                break;
            }
            case DisplaySensorRequest:
            {
                DisplaypSensorRequest(&com2Device, &request.sensorRequest, &sensorDataBuffer);
                break;
            }
            case DisplayTrainArrivalRequest:
            {
                DisplaypTrainArrivalRequest(&com2Device, &arrivalRequestBuffer, &request.arrivalRequest);
                break;
            }
            case DisplayTrainLocationRequest:
            {
                for (UINT i = 0; i < (sizeof(trainLocationIndex)/sizeof(trainLocationIndex[0])); i++)
                {
                    INT trainId = request.locationRequest.trainData->trainId;
                    if (trainLocationIndex[i] == 0 || trainLocationIndex[i] == trainId)
                    {
                        trainLocationIndex[i] = trainId;
                        DisplaypTrainLocationRequest(&com2Device, i, &request.locationRequest);
                        break;
                    }
                }
                break;
            }
            case DisplayShutdownRequest:
            {
                WriteString(&com2Device, CURSOR_CLEAR);
                running = FALSE;
                break;
            }
        }

        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

        WriteCursorPosition(&com2Device, &cursor);
    }
}

VOID
DisplayCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority10, DisplaypTask)));
}

VOID
ShowKeyboardChar (
        IN CHAR c
    )
{
    DISPLAY_REQUEST request;
    request.type = DisplayCharRequest;
    request.commandLineChar = c;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
ShowClockTime (
        IN INT clockTicks
    )
{
    DISPLAY_REQUEST request;
    request.type = DisplayClockRequest;
    request.clockTicks = clockTicks;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
ShowIdleTime (
        IN INT idlePercentage
    )
{
    DISPLAY_REQUEST request;
    request.type = DisplayIdleRequest;
    request.idlePercentage = idlePercentage;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
Log (
        IN STRING message,
        ...
    )
{
    DISPLAY_LOG_REQUEST logRequest;
    RtMemset(logRequest.message, sizeof(logRequest.message), 0);

    VA_LIST va;
    VA_START(va, message);
    INT written = RtStrPrintFormattedVa(logRequest.message, sizeof(logRequest.message), message, va);
    VA_END(va);

    ASSERT(written < sizeof(logRequest.message));
    UNREFERENCED_PARAMETER(written);

    logRequest.length = written;

    DISPLAY_REQUEST request;
    request.type = DisplayLogRequest;
    request.logRequest = &logRequest;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
ShowSwitchDirection (
        IN INT idx,
        IN INT number,
        IN CHAR direction
    )
{
    DISPLAY_SWITCH_REQUEST switchRequest = { idx, number, direction };
    DISPLAY_REQUEST request;
    request.type = DisplaySwitchRequest;
    request.switchRequest = switchRequest;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
ShowSensorStatus (
        IN SENSOR_DATA data
    )
{
    DISPLAY_SENSOR_REQUEST sensorRequest = { data };
    DISPLAY_REQUEST request;
    request.type = DisplaySensorRequest;
    request.sensorRequest = sensorRequest;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
ShowTrainArrival (
        IN UINT trainId,
        IN TRACK_NODE* node,
        IN INT tick,
        IN INT distance
    )
{
    DISPLAY_REQUEST request;
    request.type = DisplayTrainArrivalRequest;
    request.arrivalRequest.trainId = trainId;
    request.arrivalRequest.node = node;
    request.arrivalRequest.tick = tick;
    request.arrivalRequest.distance = distance;

    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}

VOID
ShowTrainLocation (
        IN TRAIN_DATA* trainData
    )
{
    DISPLAY_REQUEST request;
    request.type = DisplayTrainLocationRequest;
    request.locationRequest.trainData = trainData;
    VERIFY(SUCCESSFUL(DisplaypSendRequest(&request)));
}
