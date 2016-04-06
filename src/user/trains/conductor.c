#include "conductor.h"

#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtkernel.h>
#include <rtos.h>

#include <user/io.h>

#include "location_server.h"

#define CONDUCTOR_REVERSE_DELAY_NAME "conductor_reverse_delay"
#define CONDUCTOR_SERVER_NAME "conductor"

typedef enum CONDUCTOR_REQUEST_TYPE {
    ConductorSetTrainSpeedRequest = 0,
    ConductorReverseTrainRequest,
    ConductorSetSwitchDirectionRequest,
    ConductorRegisterWorkerRequest,
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
ConductorServerpWorkerTask()
{
    CONDUCTOR_REQUEST registerRequest;
    registerRequest.type = ConductorRegisterWorkerRequest;

    INT conductorServerId = MyParentTid();
    ASSERT(SUCCESSFUL(conductorServerId));

    while (1)
    {
        CONDUCTOR_REQUEST workRequest;

        VERIFY(SUCCESSFUL(Send(conductorServerId, &registerRequest, sizeof(registerRequest), &workRequest, sizeof(workRequest))));

        switch(workRequest.type)
        {
            case ConductorSetTrainSpeedRequest:
            {
                CONDUCTOR_SET_TRAIN_SPEED_REQUEST* setTrainSpeedRequest = &workRequest.setTrainSpeedRequest;

                VERIFY(SUCCESSFUL(TrainSetSpeed(setTrainSpeedRequest->trainId, setTrainSpeedRequest->trainSpeed)));
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(setTrainSpeedRequest->trainId, setTrainSpeedRequest->trainSpeed)));
                break;
            }

            case ConductorReverseTrainRequest:
            {
                CONDUCTOR_REVERSE_TRAIN_REQUEST* reverseTrainRequest = &workRequest.reverseTrainRequest;

                VERIFY(SUCCESSFUL(TrainSetSpeed(reverseTrainRequest->trainId, 0)));
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(reverseTrainRequest->trainId, 0)));

                INT delayTicks = reverseTrainRequest->initialTrainSpeed / 3;
                VERIFY(SUCCESSFUL(Delay(delayTicks * 100)));

                VERIFY(SUCCESSFUL(TrainReverse(reverseTrainRequest->trainId)));
                VERIFY(SUCCESSFUL(LocationServerTrainDirectionReverse(reverseTrainRequest->trainId)));

                VERIFY(SUCCESSFUL(TrainSetSpeed(reverseTrainRequest->trainId, reverseTrainRequest->initialTrainSpeed)));
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(reverseTrainRequest->trainId, reverseTrainRequest->initialTrainSpeed)));
                break;
            }

            case ConductorSetSwitchDirectionRequest:
            {
                CONDUCTOR_SET_SWITCH_DIRECTION_REQUEST* setSwitchDirectionRequest = &workRequest.setSwitchDirectionRequest;
                VERIFY(SUCCESSFUL(SwitchSetDirection(setSwitchDirectionRequest->switchId, setSwitchDirectionRequest->switchDirection)));
                VERIFY(SUCCESSFUL(LocationServerSwitchUpdated()));
                break;
            }

            default:
            {
                ASSERT(FALSE);
            }
        }
    }
}

static
VOID
ConductorServerpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(CONDUCTOR_SERVER_NAME)));

    INT conductorWorkers[10];
    RT_CIRCULAR_BUFFER conductorWorkersQueue;
    RtCircularBufferInit(&conductorWorkersQueue, conductorWorkers, sizeof(conductorWorkers));

    for (UINT i = 0; i < 10; i++)
    {
        VERIFY(SUCCESSFUL(Create(Priority14, ConductorServerpWorkerTask)));
    }

    BOOLEAN running = TRUE;
    while (running)
    {
        INT senderId;
        CONDUCTOR_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch (request.type)
        {
            case ConductorRegisterWorkerRequest:
            {
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&conductorWorkersQueue, &senderId, sizeof(senderId))));

                break;
            }

            default:
            {
                INT workerId = -1;
                if (RT_SUCCESS(RtCircularBufferPeekAndPop(&conductorWorkersQueue, &workerId, sizeof(workerId))))
                {
                    VERIFY(SUCCESSFUL(Reply(workerId, &request, sizeof(request))));
                }
                else
                {
                    Log("All our conductor workers are busy, sorry!");
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }
        }
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
