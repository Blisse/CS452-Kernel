#include "conductor.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>

#include <user/io.h>

#include "location_server.h"

#define CONDUCTOR_SERVER_NAME "conductor"

typedef enum CONDUCTOR_REQUEST_TYPE {
    ConductorSetTrainSpeedRequest = 0,
    ConductorReverseTrainRequest,
    ConductorSetSwitchDirectionRequest,
} CONDUCTOR_REQUEST_TYPE;

typedef struct CONDUCTOR_SET_TRAIN_SPEED_REQUEST {
    INT trainId;
    INT trainSpeed;
} CONDUCTOR_SET_TRAIN_SPEED_REQUEST;

typedef struct CONDUCTOR_REVERSE_TRAIN_REQUEST {
    INT trainId;
    INT initialTrainSpeed;
} CONDUCTOR_REVERSE_TRAIN_REQUEST;

typedef struct CONDUCTOR_SET_SWITCH_DIRECTION_REQUEST {
    INT switchId;
    INT switchDirection;
} CONDUCTOR_SET_SWITCH_DIRECTION_REQUEST;

typedef struct CONDUCTOR_REQUEST {
    CONDUCTOR_REQUEST_TYPE type;

    union {
        CONDUCTOR_SET_TRAIN_SPEED_REQUEST setTrainSpeedRequest;
        CONDUCTOR_REVERSE_TRAIN_REQUEST reverseTrainRequest;
        CONDUCTOR_SET_SWITCH_DIRECTION_REQUEST setSwitchDirectionRequest;
    };
} CONDUCTOR_REQUEST;

static
VOID
ConductorServerpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(CONDUCTOR_SERVER_NAME)));

    BOOLEAN running = TRUE;
    while (running)
    {
        INT senderId;
        CONDUCTOR_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch(request.type)
        {
            case ConductorSetTrainSpeedRequest:
            {
                CONDUCTOR_SET_TRAIN_SPEED_REQUEST* setTrainSpeedRequest = &request.setTrainSpeedRequest;
                VERIFY(SUCCESSFUL(TrainSetSpeed(setTrainSpeedRequest->trainId, setTrainSpeedRequest->trainSpeed)));
                Log("update loc server speed");
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(setTrainSpeedRequest->trainId, setTrainSpeedRequest->trainSpeed)));
                break;
            }

            case ConductorReverseTrainRequest:
            {
                CONDUCTOR_REVERSE_TRAIN_REQUEST* reverseTrainRequest = &request.reverseTrainRequest;
                VERIFY(SUCCESSFUL(TrainReverse(reverseTrainRequest->trainId)));
                Log("update loc server direction");
                VERIFY(SUCCESSFUL(LocationServerTrainDirectionReverse(reverseTrainRequest->trainId)));
                break;
            }

            case ConductorSetSwitchDirectionRequest:
            {
                CONDUCTOR_SET_SWITCH_DIRECTION_REQUEST* setSwitchDirectionRequest = &request.setSwitchDirectionRequest;
                VERIFY(SUCCESSFUL(SwitchSetDirection(setSwitchDirectionRequest->switchId, setSwitchDirectionRequest->switchDirection)));
                Log("update loc server switch");
                VERIFY(SUCCESSFUL(LocationServerSwitchUpdated()));
                Log("actually done");
                break;
            }
        }
        Log("finish conductor bsns");

        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

    }
}

INT
ConductorpSendMessage(
        IN CONDUCTOR_REQUEST* request
    )
{
    INT status = WhoIs(CONDUCTOR_SERVER_NAME);

    if (SUCCESSFUL(status))
    {
        INT conductorServerId = status;
        status = Send(conductorServerId, request, sizeof(*request), NULL, 0);
    }

    return status;
}

INT
ConductorSetTrainSpeed(
        IN INT trainId,
        IN INT trainSpeed
    )
{
    CONDUCTOR_REQUEST request;
    request.type = ConductorSetTrainSpeedRequest;
    request.setTrainSpeedRequest.trainId = trainId;
    request.setTrainSpeedRequest.trainSpeed = trainSpeed;

    return ConductorpSendMessage(&request);
}

INT
ConductorReverseTrain(
        IN INT trainId,
        IN INT initialTrainSpeed
    )
{
    CONDUCTOR_REQUEST request;
    request.type = ConductorReverseTrainRequest;
    request.reverseTrainRequest.trainId = trainId;
    request.reverseTrainRequest.initialTrainSpeed = initialTrainSpeed;

    return ConductorpSendMessage(&request);
}

INT
ConductorSetSwitchDirection(
        IN INT switchId,
        IN SWITCH_DIRECTION switchDirection
    )
{
    CONDUCTOR_REQUEST request;
    request.type = ConductorSetSwitchDirectionRequest;
    request.setSwitchDirectionRequest.switchId = switchId;
    request.setSwitchDirectionRequest.switchDirection = switchDirection;

    return ConductorpSendMessage(&request);
}

VOID
ConductorServerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority17, ConductorServerpTask)));
}
